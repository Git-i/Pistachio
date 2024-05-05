#include "ptpch.h"
#include "ShaderAssetCompiler.h"
#include <string_view>
#include <sstream>
#include <charconv>
#include "Pistachio\Utils\PlatformUtils.h"
namespace Pistachio
{
	IDxcLibrary*  ShaderAssetCompiler::library  = nullptr;
	IDxcCompiler* ShaderAssetCompiler::compiler = nullptr;
	static inline bool starts_with(const std::string_view& view, const char* string)
	{
		size_t length = strlen(string);
		return strncmp(view.data(), string, length) == 0;
	}
	static inline bool ends_with(const std::string_view& view, const char* string)
	{
		size_t length = strlen(string);
		return strncmp(view.data() + view.size() - length, string, length) == 0;
	}
	static inline void ltrim(std::string& s) {
		s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) {
			return !std::isspace(ch);
			}));
	}
	static inline void rtrim(std::string& s) {
		s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) {
			return !std::isspace(ch);
			}).base(), s.end());
	}
	static inline void trim(std::string& s)
	{
		ltrim(s), rtrim(s);
	}
	static std::array<const char*, 8> bindingTypes = { 
		"Texture1D", 
		"Texture1DArray", 
		"Texture2D",
		"Texture2DArray",
		"Texture3D", 
		"TextureCube",
		"SamplerState",
		"SamplerComparisonState"
	};
	
	class Words
	{
	public:
		Words(std::string& str)
		{
			size_t pos = 0;
			size_t old_pos = 0;
			while (pos = str.find(' ', pos), pos != std::string::npos)
			{
				std::string_view view(str.c_str() + old_pos, pos - old_pos);
				if (std::any_of(view.begin(), view.end(), [](char c) {return !std::isspace(c); }))
				{
					views.emplace_back(str.c_str() + old_pos, pos - old_pos);
				}
				std::cout << pos << std::endl;
				pos++;
				old_pos = pos;
			}
			views.emplace_back(str.c_str() + old_pos, str.length() - old_pos);
		}
		std::string_view operator[](uint32_t index) { return views[index]; }
		uint32_t GetCount() { return (uint32_t)views.size(); }
	private:
		std::vector<std::string_view> views;
	};
	uint32_t ShaderAssetCompiler::FindSection(const char* name)
	{
		std::string section_name = "[" + std::string(name) + "]";
		for (uint32_t i = 0; i < lines.size(); i++)
		{
			if (lines[i] == section_name) return i + 1;
		}
		return UINT32_MAX;
	}
	uint32_t ShaderAssetCompiler::GetSectionLineCount(const char* sectionName)
	{
		return GetSectionLineCount(FindSection(sectionName));
	}
	static inline bool IsSection(const std::string& str)
	{
		return starts_with(str, "[") && ends_with(str, "]");
	}
	uint32_t ShaderAssetCompiler::GetSectionLineCount(uint32_t section)
	{
		uint32_t numLines = 0;
		for (; section < lines.size(); section++)
		{
			std::string temp = lines[section];
			trim(temp);
			if (IsSection(temp)) break;
			numLines++;
		}
		return numLines;
	}
	
	uint32_t ShaderAssetCompiler::Compile(const char* filename, const char* outfileName)
	{
		if (!library || !compiler)
		{
			errorBuf = "Shader Compiler was not properly initialized";
			return 1;
		}
		type = ShaderType::Invalid;
		std::ifstream file(filename);
		std::string file_line;
		std::stringstream ostream;
		std::ofstream outFile(outfileName, std::ios::binary | std::ios::trunc);

		while (std::getline(file, file_line))
		{
			lines.push_back(file_line);
		}
		//parse Input Section
		uint32_t inputs = FindSection("Inputs");
		if (inputs == UINT32_MAX)
		{
			errorBuf = "Inputs Section Not Found";
			return 1;
		}
		std::string cbuffer;
		uint32_t inputLines = GetSectionLineCount(inputs);
		ostream << "#include <PistachioShaderCommon.hlsli>\n";
		uint32_t currentCBOffset = 0;
		//data is packed in 16 byte strides
		for (uint32_t i = inputs; i < inputLines + inputs; i++)
		{
			std::string& line = lines[i];
			if (auto commentPos = line.find("//"); commentPos != std::string::npos)
			{
				line.erase(commentPos, line.size() - commentPos);
			}
			trim(line);
			Words words(line);
			if (words[0].compare("param") == 0)
			{
				if (words.GetCount() != 4)
				{
					errorBuf = "Invalid param declaration at line" + std::to_string(i + 1);
					return 1;
				}
				if (words[2].compare(":") != 0)
				{
					errorBuf = "Unexpected token " + std::string(words[2]) + " at line" + std::to_string(i + 1);
					return 1;
				}
				//matrix types are not supported, only (float,int,uint)(2->4)
				std::string_view type(words[3]);
				uint32_t numComps = 1;
				if (std::isdigit((unsigned char)words[3][words[3].size() - 1]))
				{
					numComps = atoi(words[3].data() + words[3].size() - 1);
					if (numComps > 4 || numComps < 2)
					{
						errorBuf = "Invalid param type at line " + std::to_string(i + 1);
					}
					type = std::string_view(words[3].data(), words[3].size() - 1);
				}
				if (type.compare("float") != 0 &&
					type.compare("int") != 0 &&
					type.compare("uint") != 0)
				{
					errorBuf = "Invalid param type at line " + std::to_string(i + 1);
					return 1;
				}
				uint32_t paramType = numComps - 1;
				if (type.compare("uint") == 0) paramType += 0;
				if (type.compare("int") == 0) paramType += 4;
				if (type.compare("float") == 0) paramType += 8;
				uint32_t nextElement = (currentCBOffset / 16u + 1) * 16u;
				uint32_t currentElementSize = numComps * 4;
				//if it fits
				if ((nextElement - currentCBOffset) >= currentElementSize)
				{
					paramsOffset[std::string(words[1])] = { currentCBOffset , (ParamType)paramType};
					currentCBOffset += currentElementSize;
				}
				else
				{
					paramsOffset[std::string(words[1])] = { nextElement,(ParamType)paramType };
					currentCBOffset = nextElement + currentElementSize;
				}
				cbuffer += "\t";
				cbuffer += words[3];
				cbuffer += " ";
				cbuffer += words[1];
				cbuffer += ";\n";
				continue;
			}
			auto tex = std::find_if(bindingTypes.begin(), bindingTypes.end(), [&](const char* str) {
				return words[0].compare(str) == 0;
				});
			//this line is a texture binding
			if (tex != bindingTypes.end())
			{
				uint32_t slot;
				if (words.GetCount() != 4)
				{
					errorBuf = "Invalid binding declaration at line " + std::to_string(i + 1);
					return 1;
				}
				if (words[2].compare(":") != 0)
				{
					errorBuf = "Unexpected token " + std::string(words[2]) + " at line" + std::to_string(i + 1);
					return 1;
				}
				//extract slot number
				if (!starts_with(words[3], "slot(") || !ends_with(words[3], ")"))
				{
					errorBuf = "Invalid Slot declaration at line " + std::to_string(i + 1);
					return 1;
				}
				std::string_view number_str = words[3].substr(5, words[3].size() - 6);
				auto [ptr, ec] = std::from_chars(number_str.data(), number_str.data() + number_str.size(), slot);
				if (ptr != number_str.data() + number_str.size())
				{
					errorBuf = "Invalid Slot declaration at line " + std::to_string(i + 1);
					return 1;
				}
				ostream << words[0] << " " << words[1] << " : register(t" << slot << ", space3);\n";
				continue;
			}
			errorBuf =
				"Unexpected token " +
				std::string(words[0].data(), words[0].size()) +
				" at line " +
				std::to_string(i + 1);
			return 1;
		}
		if (!cbuffer.empty())
		{
			ostream << "cbuffer params_buffer : register(b0, space4)\n{\n" << cbuffer << "};\n";
		}
		uint32_t shaderType = FindSection("ShaderType");
		uint32_t shaderTypeLines = GetSectionLineCount(shaderType);
		if (shaderType == UINT32_MAX)
		{
			errorBuf = "ShaderType Section not found";
			return 1;
		}
		for (uint32_t i = shaderType; i < shaderType + shaderTypeLines; i++)
		{
			std::string& line = lines[shaderType];
			if (auto commentPos = line.find("//"); commentPos != std::string::npos)
			{
				line.erase(commentPos, line.size() - commentPos);
			}
			trim(line);
			if (line == "LitShader")
			{
				if (type == ShaderType::Invalid) type = ShaderType::Lit;
				else { errorBuf = "Shader Cannot have more than one type"; return 1; }
			}
			else if (line == "UnlitShader")
			{
				if (type == ShaderType::Invalid) type = ShaderType::Unlit;
				else { errorBuf = "Shader Cannot have more than one type"; return 1; }
			}
			else if (line == "CustomLitShader")
			{
				if (type == ShaderType::Invalid) type = ShaderType::CustomLit;
				else { errorBuf = "Shader Cannot have more than one type"; return 1; }
			}
			else
			{
				errorBuf = "Invalid ShaderType on line " +
					std::to_string(i + 1) +
					". Supported types are \'LitShader\',\'UnlitShader\' and \'CustomLitShader\'";
				return 1;
			}
		}
		uint32_t code = FindSection("Code");
		uint32_t codeLines = GetSectionLineCount(code);
		if (code == UINT32_MAX)
		{
			errorBuf = "Code Section not found";
			return 1;
		}
		for (uint32_t i = code; i < code + codeLines; i++)
		{
			ostream << lines[i] << '\n';
		}
		switch (type)
		{
		case Pistachio::ShaderType::Lit:
			break;
		case Pistachio::ShaderType::Unlit:
		{
			ostream << "\nfloat4 main()\n {\nreturn Output();\n}";
			break;
		}
		case Pistachio::ShaderType::CustomLit:
		{
			ostream << "\nfloat4 main()\n {\nreturn Output();\n}";
			break;
		}
		default:
			break;
		}
		//by now we should (hopefully) have a valid hlsl shader to compile
		/*
		* The Output (Compiled) File Should Look Like:
		* number of params (np),
		* str-size, param-name, param offset, param type [np-times]
		* number of bindings (nb),
		* str-size, binding-name, binding index [nb-times]
		* spirv-code size, spirv-code, dxil-code size, dxil-code
		*/
		uint32_t paramsOffsetSize = Pistachio::Edian::ConvertToBigEndian((uint32_t)paramsOffset.size());
		outFile.write((const char*)&paramsOffsetSize, sizeof(uint32_t));
		for (auto& [param, info] : paramsOffset)
		{
			uint32_t size = Pistachio::Edian::ConvertToBigEndian((uint32_t)param.size());
			outFile.write((const char*)&size, sizeof(uint32_t));
			outFile.write(param.c_str(), param.size());
			uint32_t offset = Pistachio::Edian::ConvertToBigEndian(info.offset);
			outFile.write((const char*)&offset, sizeof(uint32_t));
			uint32_t type = Pistachio::Edian::ConvertToBigEndian((uint32_t)info.type);
			outFile.write((const char*)&type, sizeof(uint32_t));
		}
		uint32_t textureIndexSize = Pistachio::Edian::ConvertToBigEndian((uint32_t)textureIndex.size());
		outFile.write((const char*)&textureIndexSize, sizeof(uint32_t));
		for (auto& [texture, index] : textureIndex)
		{
			uint32_t size = Pistachio::Edian::ConvertToBigEndian((uint32_t)texture.size());
			outFile.write((const char*)&size, sizeof(uint32_t));
			outFile.write(texture.c_str(), texture.size());
			uint32_t indexEdian = Pistachio::Edian::ConvertToBigEndian(index);
			outFile.write((const char*)&indexEdian, sizeof(uint32_t));
		}
		IDxcBlobEncoding* blob;
		library->CreateBlobWithEncodingFromPinned(ostream.str().c_str(), (uint32_t)ostream.str().size(), CP_UTF8, &blob);
		wchar_t filename_ws[512];
		mbstowcs(filename_ws, filename, 511);
		//spirv compile
		const wchar_t* args[] = { L"-spirv" };
		IDxcOperationResult* result = nullptr;
		HRESULT hr = compiler->Compile(blob, filename_ws, L"main", L"ps_6_0", args, 1, nullptr, 0, nullptr, &result);
		if (FAILED(hr))
		{
			if (result)
			{
				IDxcBlobEncoding* errorsBlob;
				hr = result->GetErrorBuffer(&errorsBlob);
				if (SUCCEEDED(hr) && errorsBlob)
				{
					errorBuf = "Code Compilation Failed";
					errorBuf += (const char*)errorsBlob->GetBufferPointer();
					errorsBlob->Release();
					return 1;
				}
			}
			errorBuf = "Code Compilation Failed with Unknown Error";
			return 1;
		}
		result->GetStatus(&hr);
		IDxcBlob* spvCode;
		result->GetResult(&spvCode);
		uint32_t spirvSize = Pistachio::Edian::ConvertToBigEndian((uint32_t)spvCode->GetBufferSize());
		outFile.write((char*)&spirvSize, sizeof(uint32_t));
		outFile.write((char*)spvCode->GetBufferPointer(), spvCode->GetBufferSize());
		spvCode->Release();
		hr = compiler->Compile(blob, filename_ws, L"main", L"ps_6_0", nullptr, 0, nullptr, 0, nullptr, &result);
		if (FAILED(hr))
		{
			if(result)
			{
				IDxcBlobEncoding* errorsBlob;
				hr = result->GetErrorBuffer(&errorsBlob);
				if (SUCCEEDED(hr) && errorsBlob)
				{
					errorBuf = "Code Compilation Failed";
					errorBuf += (const char*)errorsBlob->GetBufferPointer();
					errorsBlob->Release();
					return 1;
				}
			}
			errorBuf = "Code Compilation Failed with Unknown Error";
			return 1;
		}
		IDxcBlob* dxilCode;
		result->GetResult(&dxilCode);
		uint32_t dxilSize = (uint32_t)dxilCode->GetBufferSize();
		outFile.write((char*)&dxilSize, sizeof(uint32_t));
		outFile.write((char*)dxilCode->GetBufferPointer(), dxilSize);
		dxilCode->Release();
		return 0;
	}
}
