#pragma once
#include "../Core.h"
#include "Pistachio/Renderer/RendererBase.h"
namespace Pistachio {
	class ConstantBuffer {
	public:
		void Update(void* data, unsigned int size);
		void Create(void* data, unsigned int size);
		~ConstantBuffer() { if (pBuffer)while (pBuffer->Release()) {}; pBuffer = NULL; }
	private:
		ID3D11Buffer* pBuffer;
		friend class Shader;
	};
	struct MaterialStruct
	{
		DirectX::XMVECTOR albedo = {0,0,0};
		float metallic= 0;
		float roughness=0;
		int ID = 0;
	};
	enum class ShaderType
	{
		Vertex = 0,
		Pixel = 1,
		Fragment = 1,
		Geometry = 2
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
		BufferLayout();
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


	class GeometryShader {
	public:
		GeometryShader(const wchar_t* src);
		GeometryShader();
		void Bind();
		void Shutdown();
		~GeometryShader();
	private:
		#ifdef PISTACHIO_RENDER_API_DX11
		ID3DBlob* pBlob = NULL;
		ID3D11GeometryShader* pGeometryShader = NULL;
		#endif // PISTACHIO_RENDER_API_DX11
		friend class Shader;
	};

	class PixelShader {
	public:
		PixelShader(const wchar_t* src);
		PixelShader();
		void Bind();
		void Shutdown();
		~PixelShader();
	private:
		#ifdef PISTACHIO_RENDER_API_DX11
		ID3DBlob* pBlob = NULL;
		ID3D11PixelShader* pPixelShader = NULL;
		#endif // PISTACHIO_RENDER_API_DX11
		friend class Shader;
	};

	class VertexShader {
	public:
		VertexShader(const wchar_t* src);
		VertexShader();
		void Bind();
		void Shutdown();
		~VertexShader();
	private:
		#ifdef PISTACHIO_RENDER_API_DX11
		ID3DBlob* pBlob = NULL;
		ID3D11VertexShader* pVertexShader = NULL;
		#endif // PISTACHIO_RENDER_API_DX11
		friend class Shader;
	};

	class Shader
	{
	public:
		Shader(const wchar_t* vsrc, const wchar_t* fsrc);
		Shader(const wchar_t* vsrc, const wchar_t* fsrc, const wchar_t* gsrc);
		Shader(const PixelShader& ps, const VertexShader& vs, const GeometryShader& gs);
		Shader(const PixelShader& ps, const VertexShader& vs);
		void CreateLayout(BufferLayout* layout, int nAttributes);
		void Bind(ShaderType type);
		static void SetVSBuffer(const ConstantBuffer& buffer, int startslot = 0);
		static void SetPSBuffer(const ConstantBuffer& buffer, int startslot = 0);
		static void SetGSBuffer(const ConstantBuffer& buffer, int startslot = 0);
		void Shutdown();
		~Shader();
	private:
		PixelShader m_ps;
		VertexShader m_vs;
		GeometryShader m_gs;
		ID3D11InputLayout* pInputLayout;
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

