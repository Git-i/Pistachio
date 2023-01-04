#include "ptpch.h"
#include "Shader.h"
#include "DirectX11/DX11Shader.h"
namespace Pistachio {
	Shader::Shader(const wchar_t* vsrc, const wchar_t* fsrc)
	{
		#ifdef PISTACHIO_RENDER_API_DX11
			pPixelShader = DX11Shader::CreatePixelShader(fsrc, &pBlob);
			pVertexShader = DX11Shader::CreateVertexShader(vsrc, &pBlob);
		#endif // PISTACHIO_RENDER_API_DX11
	}
	void Shader::CreateLayout(BufferLayout* layout, int nAttributes)
	{
		#ifdef PISTACHIO_RENDER_API_DX11
			pInputLayout = DX11Shader::CreateInputLayout(layout, nAttributes, pBlob->GetBufferPointer(), pBlob->GetBufferSize());
		#endif // PISTACHIO_RENDER_API_DX11

	}
	void Shader::Bind(ShaderType type)
	{
		#ifdef PISTACHIO_RENDER_API_DX11
			RendererBase::Getd3dDeviceContext()->IASetInputLayout(pInputLayout);
			if (type == ShaderType::Vertex) Pistachio::RendererBase::Getd3dDeviceContext()->VSSetShader(pVertexShader, nullptr, 0);
			else if (type == ShaderType::Fragment) Pistachio::RendererBase::Getd3dDeviceContext()->PSSetShader(pPixelShader, nullptr, 0);
		#endif // PISTACHIO_RENDER_API_DX11

	}
	void Shader::SetUniformBuffer(const ConstantBuffer& cb, int slot)
	{
		DX11Shader::CreateConstantBuffer(cb, RendererBase::Getd3dDevice(), RendererBase::Getd3dDeviceContext(), slot);
	}
	void Shader::SetVSRandomBuffer(const void* cb, int size, int slot)
	{
		DX11Shader::CreateRandomConstantBuffer(cb, size, RendererBase::Getd3dDevice(), RendererBase::Getd3dDeviceContext(), slot);
	}
	void Shader::SetPSBuffer(const void* cb, int size, int slot)
	{
		DX11Shader::CreatePSRandomConstantBuffer(cb, size, RendererBase::Getd3dDevice(), RendererBase::Getd3dDeviceContext(), slot);
	}
	void Shader::Shutdown()
	{
		if(pBlob != NULL)
			pBlob->Release();
		if(pVertexShader != NULL)
			pVertexShader->Release();
		if(pPixelShader != NULL)
			pPixelShader->Release();
		if(pInputLayout != NULL)
			pInputLayout->Release();
	}
	Shader::~Shader()
	{
		Shutdown();
	}
	void ShaderLibrary::Add(const std::string& name, const Ref<Shader>& Shader)
	{
		PT_CORE_ASSERT(m_Shaders.find(name)==m_Shaders.end());
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
		PT_CORE_ASSERT(m_Shaders.find(name)!=m_Shaders.end());
		return m_Shaders[name];
	}
}