#pragma once
#include "rhi_sc.h"
#include <memory>
#include <unordered_map>
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
			if (!compiler)
			{
				compiler = RHI::ShaderCompiler::Compiler::New();
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
		static std::unique_ptr<RHI::ShaderCompiler::Compiler> compiler;
		uint32_t FindSection(const char* sectionName);
		uint32_t GetSectionLineCount(const char* sectionName);
		uint32_t GetSectionLineCount(uint32_t sectionOffset);
	};
}
