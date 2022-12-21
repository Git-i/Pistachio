#pragma once
#include "../Core.h"
namespace Pistachio {
	struct ConstantBuffer
	{
		DirectX::XMMATRIX cameraTransform;
		DirectX::XMMATRIX transform;
		DirectX::XMVECTOR CamPos;
		DirectX::XMVECTOR albedo = {0,0,0}; //(float3) my_texture.Sample(my_sampler, uv);
		float metallic= 0;
		float roughness=0;
		float ao=0;
	};
	enum class ShaderType
	{
		Vertex = 0,
		Pixel = 1,
		Fragment = 1
	};
	enum class BufferLayoutFormat
	{
		FLOAT4 = 0,
		UINT4,
		INT4,
		FLOAT3,
		UINT3,
		INT3,
		FLOAT2,
		UINT2,
		INT2,
		FLOAT,
		UINT,
		INT,
	};
	struct BufferLayout
	{
		const char* Name;
		BufferLayoutFormat Format;
		unsigned int Offset;

		BufferLayout(const char* name, BufferLayoutFormat format, unsigned int offset) : Name(name), Format(format), Offset(offset)
		{}
	};
	static unsigned int BufferLayoutFormatSize(BufferLayoutFormat format)
	{
		switch (format)
		{
		case BufferLayoutFormat::FLOAT4: return 4 * 4;
		case BufferLayoutFormat::UINT4:  return 4 * 4;
		case BufferLayoutFormat::INT4:   return 4 * 4;
		case BufferLayoutFormat::FLOAT3: return 4 * 3;
		case BufferLayoutFormat::UINT3:  return 4 * 3;
		case BufferLayoutFormat::INT3:   return 4 * 3;
		case BufferLayoutFormat::FLOAT2: return 4 * 2;
		case BufferLayoutFormat::UINT2:  return 4 * 2;
		case BufferLayoutFormat::INT2:   return 4 * 2;
		case BufferLayoutFormat::FLOAT:  return 4;
		case BufferLayoutFormat::UINT:   return 4;
		case BufferLayoutFormat::INT:    return 4;
		default:
			break;
		}
	}

	class Shader
	{
	public:
		Shader(const wchar_t* vsrc, const wchar_t* fsrc);
		void CreateLayout(BufferLayout* layout, int nAttributes);
		void Bind(ShaderType type);
		void SetUniformBuffer(const ConstantBuffer& cb);
		void SetRandomConstantBuffer(const void* cb, int size);
		void Shutdown();
		~Shader();
	private:
		#ifdef PISTACHIO_RENDER_API_DX11
			ID3DBlob* pBlob = NULL;
			ID3D11VertexShader* pVertexShader = NULL;
			ID3D11PixelShader* pPixelShader = NULL;
			ID3D11InputLayout* pInputLayout = NULL;
		#endif // PISTACHIO_RENDER_API_DX11
	};
	class ShaderLibrary
	{
	public:
		void Add(const std::string& name,const Ref<Shader>& Shader);
		Ref<Shader> Load(const std::string& name, const std::string& vertex, const std::string& fragment);
		Ref<Shader> Get(const std::string& name);
	private:
		std::unordered_map<std::string, Ref<Shader>> m_Shaders;
	};
}

