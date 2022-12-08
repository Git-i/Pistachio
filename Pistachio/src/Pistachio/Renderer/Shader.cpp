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
		if (type == ShaderType::Vertex) Pistachio::Renderer::g_pd3dDeviceContext->VSSetShader(pVertexShader, nullptr, 0);
		else if (type == ShaderType::Fragment) Pistachio::Renderer::g_pd3dDeviceContext->PSSetShader(pPixelShader, nullptr, 0);
#endif // PISTACHIO_RENDER_API_DX11

	}
}