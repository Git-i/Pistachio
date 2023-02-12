#include "ptpch.h"
#include "Shader.h"
#include "DirectX11/DX11Shader.h"
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
		#ifdef PISTACHIO_RENDER_API_DX11
			DX11Shader::CreateInputLayout(layout, nAttributes, m_vs.pBlob->GetBufferPointer(), m_vs.pBlob->GetBufferSize(), &pInputLayout);
		#endif // PISTACHIO_RENDER_API_DX11

	}
	void Shader::Bind(ShaderType type)
	{
		PT_PROFILE_FUNCTION()
		#ifdef PISTACHIO_RENDER_API_DX11
			RendererBase::Getd3dDeviceContext()->IASetInputLayout(pInputLayout);
			if (type == ShaderType::Vertex) Pistachio::RendererBase::Getd3dDeviceContext()->VSSetShader(m_vs.pVertexShader, nullptr, 0);
			else if (type == ShaderType::Fragment) Pistachio::RendererBase::Getd3dDeviceContext()->PSSetShader(m_ps.pPixelShader, nullptr, 0);
			else if (type == ShaderType::Geometry) Pistachio::RendererBase::Getd3dDeviceContext()->GSSetShader(m_gs.pGeometryShader, nullptr, 0);
		#endif // PISTACHIO_RENDER_API_DX11
	}
	void Shader::Shutdown()
	{
		m_ps.Shutdown();
		m_vs.Shutdown();
		m_gs.Shutdown();
		if (pInputLayout) { while (pInputLayout->Release()) {}; pInputLayout = NULL; };
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
		RendererBase::Getd3dDeviceContext()->Map(pBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
		memcpy(mappedResource.pData, data, size);
		RendererBase::Getd3dDeviceContext()->Unmap(pBuffer, 0);
	}
	void ConstantBuffer::Create(void* data, unsigned int size) {
		DX11Shader::CreateConstantBuffer(data, size, RendererBase::Getd3dDevice(), RendererBase::Getd3dDeviceContext(), &pBuffer);
	}
	void Shader::SetVSBuffer(const ConstantBuffer& buffer, int slot)
	{
		PT_PROFILE_FUNCTION()
			RendererBase::Getd3dDeviceContext()->VSSetConstantBuffers(slot, 1, &buffer.pBuffer);
	}
	void Shader::SetPSBuffer(const ConstantBuffer& buffer, int slot)
	{
		PT_PROFILE_FUNCTION()
			RendererBase::Getd3dDeviceContext()->PSSetConstantBuffers(slot, 1, &buffer.pBuffer);
	}
	void Shader::SetGSBuffer(const ConstantBuffer& buffer, int slot)
	{
		PT_PROFILE_FUNCTION()
			RendererBase::Getd3dDeviceContext()->GSSetConstantBuffers(slot, 1, &buffer.pBuffer);
	}
	GeometryShader::GeometryShader(const wchar_t* src)
	{
		PT_CORE_ASSERT(D3DReadFileToBlob(src, &pBlob) == S_OK);
		Pistachio::RendererBase::Getd3dDevice()->CreateGeometryShader(pBlob->GetBufferPointer(), pBlob->GetBufferSize(), nullptr, &pGeometryShader);
	}
	void GeometryShader::Bind()
	{
		RendererBase::Getd3dDeviceContext()->GSSetShader(pGeometryShader, nullptr, 0);
	}
	void GeometryShader::Shutdown()
	{
		if (pGeometryShader != NULL) { while (pGeometryShader->Release()) {}; pGeometryShader = NULL; }
		if (pBlob != NULL) { while (pBlob->Release()) {}; pBlob = NULL; }
	}
	GeometryShader::~GeometryShader()
	{
		Shutdown();
	}
	GeometryShader::GeometryShader() { pGeometryShader = NULL; }
	PixelShader::PixelShader(const wchar_t* src)
	{
		PT_CORE_ASSERT(D3DReadFileToBlob(src, &pBlob) == S_OK);
		Pistachio::RendererBase::Getd3dDevice()->CreatePixelShader(pBlob->GetBufferPointer(), pBlob->GetBufferSize(), nullptr, &pPixelShader);
	}
	void PixelShader::Bind()
	{
		RendererBase::Getd3dDeviceContext()->PSSetShader(pPixelShader, nullptr, 0);
	}
	void PixelShader::Shutdown()
	{
		if (pPixelShader != NULL) { while (pPixelShader->Release()) {}; pPixelShader = NULL; }
		if (pBlob != NULL) { while (pBlob->Release()) {}; pBlob = NULL; }
	}
	PixelShader::~PixelShader() {
		Shutdown();
	}
	PixelShader::PixelShader() { pPixelShader = NULL; }
	VertexShader::VertexShader(const wchar_t* src)
	{
		PT_CORE_ASSERT(D3DReadFileToBlob(src, &pBlob) == S_OK);
		Pistachio::RendererBase::Getd3dDevice()->CreateVertexShader(pBlob->GetBufferPointer(), pBlob->GetBufferSize(), nullptr, &pVertexShader);
	}
	void VertexShader::Bind()
	{
		RendererBase::Getd3dDeviceContext()->VSSetShader(pVertexShader, nullptr, 0);
	}
	VertexShader::~VertexShader()
	{
		Shutdown();
	}
	void VertexShader::Shutdown()
	{
		if (pVertexShader != NULL) { while (pVertexShader->Release()) {}; pVertexShader = NULL; }
		if (pBlob != NULL) { while (pBlob->Release()) {}; pBlob = NULL; }
	}
	VertexShader::VertexShader() { pVertexShader = NULL; }
}