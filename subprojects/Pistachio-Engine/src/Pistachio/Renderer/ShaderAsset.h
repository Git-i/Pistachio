#pragma once
#include "../Asset/RefCountedObject.h"
#include "Pistachio/Renderer/RendererContext.h"
#include "Shader.h"
namespace Pistachio
{
	enum class ParamType : uint32_t //need explicit size for binary serialization
	{
		Uint = 0,
		Uint2 = 1,
		Uint3 = 2,
		Uint4 = 3,
		Int = 4,
		Int2 = 5,
		Int3 = 6,
		Int4 = 7,
		Float = 8,
		Float2 = 9,
		Float3 = 10,
		Float4 = 11
	};
	struct ParamInfo
	{
		uint32_t offset;
		ParamType type;
	};
	/*
	* All Material's have a shader asset
	* Custom Shaders are implemented through the shader asset class
	* All Custom Shaders should have:
	*	-Their material propertied in set 2
	*	-All non texture parameters in a constant buffer, bound to slot 0
	*/
	class PISTACHIO_API ShaderAsset : public RefCountedObject
	{
	public:
		~ShaderAsset();
		static ShaderAsset* Create(const char* filename);
		ParamInfo GetParameterInfo(const std::string& paramName);
		inline Shader& GetShader() { return shader; }
		inline const Shader& GetShader() const { return shader; }
		inline uint32_t GetParamBufferSize() { return paramBufferSize; }
	private:
		friend class RendererContext;
		uint32_t paramBufferSize;
		std::unordered_map<std::string, ParamInfo> parametersMap;
		std::unordered_map<std::string, uint32_t> bindingsMap;
		static std::vector<char> vs;
		static RHI::Ptr<RHI::ShaderReflection> VSReflection;
		Shader shader;
	};
	
}
