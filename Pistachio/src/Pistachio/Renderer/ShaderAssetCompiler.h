#pragma once
#include <dxcapi.h>
namespace Pistachio
{
	enum class ShaderType
	{
		Invalid, Lit, Unlit, CustomLit 
	};
	enum class ParamType : uint32_t //need explicit size for binary serialization
	{
		Uint  = 0,
		Uint2 = 1, 
		Uint3 = 2,
		Uint4 = 3,
		Int  = 4, 
		Int2 = 5, 
		Int3 = 6,
		Int4 = 7,
		Float  = 8, 
		Float2 = 9,
		Float3 = 10,
		Float4 = 11
	};
	struct ParamInfo
	{
		uint32_t offset;
		ParamType type;
	};

	class ShaderAssetCompiler
	{
	public:
		ShaderAssetCompiler()
		{
			if (!library)
			{
				HRESULT hr = DxcCreateInstance(CLSID_DxcLibrary, IID_PPV_ARGS(&library));
				if (FAILED(hr))
				{
					PT_CORE_ERROR("Couldn't Initialize Shader Compiler");
					library = nullptr;
				}
			}
			if (!compiler)
			{
				HRESULT hr = DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&compiler));
				if (FAILED(hr))
				{
					PT_CORE_ERROR("Couldn't Initialize Shader Compiler");
					compiler = nullptr;
				}
			}
		}
		uint32_t Compile(const char* filename, const char* outfile);//0 means fail, 1 means success

	private:
		std::vector<std::string> lines;
		std::unordered_map<std::string, ParamInfo> paramsOffset;
		//binding by index is faster and still possible, but binding by name is supported and more convenient
		std::unordered_map<std::string, uint32_t> textureIndex; 
		ShaderType type;
		std::string errorBuf;
		static IDxcLibrary* library;
		static IDxcCompiler* compiler;
		uint32_t FindSection(const char* sectionName);
		uint32_t GetSectionLineCount(const char* sectionName);
		uint32_t GetSectionLineCount(uint32_t sectionOffset);
	};
}
