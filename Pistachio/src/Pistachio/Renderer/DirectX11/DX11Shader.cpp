#include "ptpch.h"
#include "../Shader.h"
#include "../RendererBase.h"
#define VERTEX_SHADER(ID) ((ID3D11VertexShader*)ID)
#define VERTEX_SHADER_PP(ID) ((ID3D11VertexShader**)&ID)

#define PIXEL_SHADER(ID) ((ID3D11PixelShader*)ID)
#define PIXEL_SHADER_PP(ID) ((ID3D11PixelShader**)&ID)

#define GEOMETRY_SHADER(ID) ((ID3D11GeometryShader*)ID)
#define GEOMETRY_SHADER_PP(ID) ((ID3D11GeometryShader**)&ID)

#define BLOB(ID) ((ID3D10Blob*)ID)
#define BLOB_PP(ID) ((ID3D10Blob**)&ID)

#define INPUT_LAYOUT(ID) ((ID3D11InputLayout*)ID)
#define INPUT_LAYOUT_PP(ID) ((ID3D11InputLayout**)&ID)

#define BUFFER(ID) ((ID3D11Buffer*)ID)
#define BUFFER_PP(ID) ((ID3D11Buffer**)&ID)

static DXGI_FORMAT DXGIFormat(Pistachio::BufferLayoutFormat format)
{
	switch (format)
	{
	case Pistachio::BufferLayoutFormat::FLOAT4: return DXGI_FORMAT_R32G32B32A32_FLOAT;
	case Pistachio::BufferLayoutFormat::UINT4: return DXGI_FORMAT_R32G32B32A32_UINT;
	case Pistachio::BufferLayoutFormat::INT4: return  DXGI_FORMAT_R32G32B32A32_SINT;
	case Pistachio::BufferLayoutFormat::FLOAT3: return DXGI_FORMAT_R32G32B32_FLOAT;
	case Pistachio::BufferLayoutFormat::UINT3: return DXGI_FORMAT_R32G32B32_UINT;
	case Pistachio::BufferLayoutFormat::INT3: return  DXGI_FORMAT_R32G32B32_SINT;
	case Pistachio::BufferLayoutFormat::FLOAT2: return DXGI_FORMAT_R32G32_FLOAT;
	case Pistachio::BufferLayoutFormat::UINT2: return DXGI_FORMAT_R32G32_UINT;
	case Pistachio::BufferLayoutFormat::INT2: return DXGI_FORMAT_R32G32_SINT;
	case Pistachio::BufferLayoutFormat::FLOAT: return DXGI_FORMAT_R32_FLOAT;
	case Pistachio::BufferLayoutFormat::UINT: return DXGI_FORMAT_R32_UINT;
	case Pistachio::BufferLayoutFormat::INT: return DXGI_FORMAT_R32_SINT;
	default:
		break;
	}
	return DXGI_FORMAT_UNKNOWN;
}

namespace Pistachio {
	Shader::Shader(const wchar_t* vsrc, const wchar_t* fsrc)
		:m_ps(fsrc), m_vs(vsrc)
	{
	}
	Shader::Shader(const PixelShader& ps, const VertexShader& vs, const GeometryShader& gs)
		:m_ps(ps), m_vs(vs), m_gs(gs)
	{
	}
	Shader::Shader(const PixelShader& ps, const VertexShader& vs)
		:m_ps(ps), m_vs(vs)
	{
	}
	Shader::Shader(const wchar_t* vsrc, const wchar_t* fsrc, const wchar_t* gsrc)
		:m_ps(fsrc), m_vs(vsrc), m_gs(gsrc)
	{
	}
	void Shader::CreateLayout(BufferLayout* layout, int nAttributes)
	{
		PT_PROFILE_FUNCTION()
			std::array<D3D11_INPUT_ELEMENT_DESC, 5> ied;
		for (int i = 0; i < nAttributes; i++) {
			ied[i] = (D3D11_INPUT_ELEMENT_DESC{ layout[i].Name, 0, DXGIFormat(layout[i].Format), 0, layout[i].Offset, D3D11_INPUT_PER_VERTEX_DATA, 0 });
		}
		Pistachio::RendererBase::Getd3dDevice()->CreateInputLayout(&ied.front(), nAttributes, BLOB(m_vs.Blob_ID)->GetBufferPointer(), BLOB(m_vs.Blob_ID)->GetBufferSize(), INPUT_LAYOUT_PP(InputLayout_ID));
	}
	void Shader::Bind(ShaderType type)
	{
		PT_PROFILE_FUNCTION()
		RendererBase::Getd3dDeviceContext()->IASetInputLayout(INPUT_LAYOUT(InputLayout_ID));
		if (type == ShaderType::Vertex) Pistachio::RendererBase::Getd3dDeviceContext()->VSSetShader(VERTEX_SHADER(m_vs.ID), nullptr, 0);
		else if (type == ShaderType::Fragment) Pistachio::RendererBase::Getd3dDeviceContext()->PSSetShader(PIXEL_SHADER(m_ps.ID), nullptr, 0);
		else if (type == ShaderType::Geometry) Pistachio::RendererBase::Getd3dDeviceContext()->GSSetShader(GEOMETRY_SHADER(m_gs.ID), nullptr, 0);
	}
	void Shader::Shutdown()
	{
		m_ps.Shutdown();
		m_vs.Shutdown();
		m_gs.Shutdown();
		if (InputLayout_ID) { while (INPUT_LAYOUT(InputLayout_ID)->Release()) {}; InputLayout_ID = NULL; };
	}
	Shader::~Shader()
	{
		Shutdown();
	}


	void ShaderLibrary::Add(const std::string& name, const Ref<Shader>& Shader)
	{
		PT_CORE_ASSERT(m_Shaders.find(name) == m_Shaders.end());
		m_Shaders[name] = Shader;
	}
	Ref<Shader> ShaderLibrary::Load(const std::string& name, const std::string& vertex, const std::string& fragment)
	{
		auto shader = std::make_shared<Shader>((wchar_t*)(vertex.c_str()), (wchar_t*)(fragment.c_str()));
		Add(name, shader);
		return shader;
	}
	Ref<Shader> ShaderLibrary::Get(const std::string& name)
	{
		PT_CORE_ASSERT(m_Shaders.find(name) != m_Shaders.end());
		return m_Shaders[name];
	}


	void ConstantBuffer::Update(void* data, unsigned int size)
	{
		D3D11_MAPPED_SUBRESOURCE mappedResource;
		ZeroMemory(&mappedResource, sizeof(D3D11_MAPPED_SUBRESOURCE));
		RendererBase::Getd3dDeviceContext()->Map(BUFFER(ID), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
		memcpy(mappedResource.pData, data, size);
		RendererBase::Getd3dDeviceContext()->Unmap(BUFFER(ID), 0);
	}
	void ConstantBuffer::Create(void* data, unsigned int size) {
		D3D11_BUFFER_DESC cbd = {};
		cbd.ByteWidth = size;
		cbd.Usage = D3D11_USAGE_DYNAMIC;
		cbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		cbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		cbd.MiscFlags = 0;
		cbd.StructureByteStride = 0;
		D3D11_SUBRESOURCE_DATA sd;
		sd.pSysMem = data;
		RendererBase::Getd3dDevice()->CreateBuffer(&cbd, &sd, BUFFER_PP(ID));
	}
	ConstantBuffer::~ConstantBuffer()
	{
		if (ID) {
			while (BUFFER(ID)->Release()) {};
			ID = NULL;
		}
	}

	void Shader::SetVSBuffer(const ConstantBuffer& buffer, int slot)
	{
		PT_PROFILE_FUNCTION()
		RendererBase::Getd3dDeviceContext()->VSSetConstantBuffers(slot, 1, BUFFER_PP(buffer.ID));
	}
	void Shader::SetPSBuffer(const ConstantBuffer& buffer, int slot)
	{
		PT_PROFILE_FUNCTION()
			RendererBase::Getd3dDeviceContext()->PSSetConstantBuffers(slot, 1, BUFFER_PP(buffer.ID));
	}
	void Shader::SetGSBuffer(const ConstantBuffer& buffer, int slot)
	{
		PT_PROFILE_FUNCTION()
			RendererBase::Getd3dDeviceContext()->GSSetConstantBuffers(slot, 1, BUFFER_PP(buffer.ID));
	}


	GeometryShader::GeometryShader(const wchar_t* src)
	{
		PT_CORE_ASSERT(D3DReadFileToBlob(src, BLOB_PP(Blob_ID)) == S_OK);
		Pistachio::RendererBase::Getd3dDevice()->CreateGeometryShader(BLOB(Blob_ID)->GetBufferPointer(), BLOB(Blob_ID)->GetBufferSize(), nullptr, GEOMETRY_SHADER_PP(ID));
	}
	void GeometryShader::Bind()
	{
		RendererBase::Getd3dDeviceContext()->GSSetShader(GEOMETRY_SHADER(ID), nullptr, 0);
	}
	void GeometryShader::Shutdown()
	{
		if (ID) { while (GEOMETRY_SHADER(ID)->Release()) {}; ID = NULL; }
		if (Blob_ID) { while (BLOB(Blob_ID)->Release()) {}; Blob_ID = NULL; }
	}
	GeometryShader::~GeometryShader()
	{
		Shutdown();
	}
	GeometryShader::GeometryShader() { ID = NULL; Blob_ID = NULL; }


	PixelShader::PixelShader(const wchar_t* src)
	{
		PT_CORE_ASSERT(D3DReadFileToBlob(src, BLOB_PP(Blob_ID)) == S_OK);
		Pistachio::RendererBase::Getd3dDevice()->CreatePixelShader(BLOB(Blob_ID)->GetBufferPointer(), BLOB(Blob_ID)->GetBufferSize(), nullptr, PIXEL_SHADER_PP(ID));
	}
	void PixelShader::Bind()
	{
		RendererBase::Getd3dDeviceContext()->PSSetShader(PIXEL_SHADER(ID), nullptr, 0);
	}
	void PixelShader::Shutdown()
	{
		if (ID) { while (PIXEL_SHADER(ID)->Release()) {}; ID = NULL; }
		if (Blob_ID) { while (BLOB(Blob_ID)->Release()) {}; Blob_ID = NULL; }
	}
	PixelShader::~PixelShader() {
		Shutdown();
	}
	PixelShader::PixelShader() { ID = NULL; Blob_ID = NULL; }


	VertexShader::VertexShader(const wchar_t* src)
	{
		PT_CORE_ASSERT(D3DReadFileToBlob(src, BLOB_PP(Blob_ID)) == S_OK);
		Pistachio::RendererBase::Getd3dDevice()->CreateVertexShader(BLOB(Blob_ID)->GetBufferPointer(), BLOB(Blob_ID)->GetBufferSize(), nullptr, VERTEX_SHADER_PP(ID));
	}
	void VertexShader::Bind()
	{
		RendererBase::Getd3dDeviceContext()->VSSetShader(VERTEX_SHADER(ID), nullptr, 0);
	}
	VertexShader::~VertexShader()
	{
		Shutdown();
	}
	void VertexShader::Shutdown()
	{
		if (ID) { while (VERTEX_SHADER(ID)->Release()) {}; ID = NULL; }
		if (Blob_ID) { while (BLOB(Blob_ID)->Release()) {}; Blob_ID = NULL; }
	}
	VertexShader::VertexShader() { ID = NULL; Blob_ID = NULL; }
}

#undef VERTEX_SHADER(ID)
#undef VERTEX_SHADER_PP(ID)
 
#undef PIXEL_SHADER(ID)
#undef PIXEL_SHADER_PP(ID)
 
#undef GEOMETRY_SHADER(ID)
#undef GEOMETRY_SHADER_PP(ID)
 
#undef BLOB(ID)
#undef BLOB_PP(ID)
 
#undef INPUT_LAYOUT(ID)
#undef INPUT_LAYOUT_PP(ID)

#undef BUFFER(ID)
#undef BUFFER_PP(ID)
