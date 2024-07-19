#pragma once
#include <string>
#include <type_traits>
#include "../Asset/Asset.h"
#include "../Asset/RefCountedObject.h"
#include "../Core/Math.h"
#include "../Asset/AssetManager.h"
#include "BufferHandles.h"
#include "ShaderAsset.h"
#include "Texture.h"
#include "Buffer.h"
#include "Renderer.h"
#include "../Core.h"
namespace Pistachio
{
	
	class PISTACHIO_API Material : public RefCountedObject
	{
	public:
		void ChangeTexture(uint32_t slot, Texture* texture);
		template<typename ParamTy> void ChangeParam(const std::string& name, const ParamTy& value);
		static Material* Create(const char* filepath);
		void SetShader(Asset shader);
		const Asset& GetShader() { return shader; }
		//Unsafe: use only if you wrote this engine or know what you're doing
		template<typename ParamTy> void ChangeParam(uint32_t size,const ParamTy* value, uint32_t offset);
		void Bind(RHI::Weak<RHI::GraphicsCommandList> list);
		RendererCBHandle parametersBuffer;
		void* parametersBufferCPU;
	public:
		friend class MaterialSerializer;
		std::vector<Asset> m_textures;
		Asset shader;
		SetInfo mtlInfo;
	};

	class PISTACHIO_API MaterialSerializer
	{
	public:
		void Serialize(const std::string& filepath, const Material& mat);
	};
	template<typename ParamTy>
	inline void Material::ChangeParam(uint32_t size, const ParamTy* value, uint32_t offset)
	{
		Renderer::PartialCBUpdate(parametersBuffer, (void*)value, offset, size);
		memcpy(((uint8_t*)parametersBufferCPU) + offset, value, size);
	}
	template<typename ParamTy, uint32_t components, typename checkTy, typename shaderTy>
	inline void CheckAndUpdate(const ParamTy& value, Pistachio::ParamInfo& info, RendererCBHandle handle,void* cpu)
	{
		PT_CORE_ASSERT(std::is_convertible_v<ParamTy, checkTy>);
		shaderTy data[components];
		if constexpr (components == 1) { data[0] = (shaderTy)value; }
		else
		{
			checkTy vector = (checkTy)value;
			for (uint32_t i = 0; i < components; i++) { data[i] = ((float*)&vector)[i]; }
		}
		Renderer::PartialCBUpdate(handle, data, info.offset, sizeof(uint32_t) * components);
		memcpy(((uint8_t*)cpu) + info.offset, data, sizeof(uint32_t) * components);
		return;
	}
	template<typename ParamTy>
	inline void Material::ChangeParam(const std::string& name, const ParamTy& value)
	{
		ShaderAsset* res = GetAssetManager()->GetShaderResource(shader);
		ParamInfo info = res->GetParameterInfo(name);
		PT_CORE_ASSERT(info.offset != UINT32_MAX);
		//single component
		switch (info.type)
		{
		case Pistachio::ParamType::Uint:
		{
			CheckAndUpdate<ParamTy, 1, uint32_t, uint32_t>(value, info, parametersBuffer,parametersBufferCPU);
			break;
		}
		case Pistachio::ParamType::Uint2:
		{
			CheckAndUpdate<ParamTy, 2, Vector2, uint32_t>(value, info, parametersBuffer, parametersBufferCPU);
			break;
		}
		case Pistachio::ParamType::Uint3:
		{
			CheckAndUpdate<ParamTy, 3, Vector3, uint32_t>(value, info, parametersBuffer, parametersBufferCPU);
			break;
		}
		case Pistachio::ParamType::Uint4:
		{
			CheckAndUpdate<ParamTy, 4, Vector4, uint32_t>(value, info, parametersBuffer, parametersBufferCPU);
			break;
		}
		case Pistachio::ParamType::Int:
		{
			CheckAndUpdate<ParamTy, 1, int32_t, int32_t>(value, info, parametersBuffer, parametersBufferCPU);
			break;
		}
		case Pistachio::ParamType::Int2:
		{
			CheckAndUpdate<ParamTy, 2, Vector2, int32_t>(value, info, parametersBuffer, parametersBufferCPU);
			break;
		}
		case Pistachio::ParamType::Int3:
		{
			CheckAndUpdate<ParamTy, 3, Vector3, int32_t>(value, info, parametersBuffer, parametersBufferCPU);
			break;
		}
		case Pistachio::ParamType::Int4:
		{
			CheckAndUpdate<ParamTy, 4, Vector4, int32_t>(value, info, parametersBuffer, parametersBufferCPU);
			break;
		}
		case Pistachio::ParamType::Float:
		{
			CheckAndUpdate<ParamTy, 1, float, float>(value, info, parametersBuffer, parametersBufferCPU);
			break;
		}
		case Pistachio::ParamType::Float2:
		{
			CheckAndUpdate<ParamTy, 2, Vector2, float>(value, info, parametersBuffer, parametersBufferCPU);
			break;
		}
		case Pistachio::ParamType::Float3:
		{
			CheckAndUpdate<ParamTy, 3, Vector3, float>(value, info, parametersBuffer, parametersBufferCPU);
			break;
		}
		case Pistachio::ParamType::Float4:
		{
			CheckAndUpdate<ParamTy, 4, Vector4, float>(value, info, parametersBuffer, parametersBufferCPU);
			break;
		}
		default:
			break;
		}
		
	}
}
