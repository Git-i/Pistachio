#include "ptpch.h"
#include "Renderer.h"
#include "Pistachio/Core/Window.h"
DirectX::XMMATRIX Pistachio::Renderer::m_viewProjectionMatrix = DirectX::XMMatrixIdentity();
namespace Pistachio {
	void Renderer::EndScene()
	{
		auto data = ((WindowData*)GetWindowDataPtr());
		RendererBase::GetSwapChain()->Present(data->vsync, 0);
	}
	void Renderer::Submit(Buffer* buffer, Shader* shader, DirectX::XMMATRIX transform)
	{
		shader->Bind(ShaderType::Vertex);
		shader->Bind(ShaderType::Pixel);
		ConstantBuffer cb;
		cb = { m_viewProjectionMatrix, DirectX::XMMatrixTranspose(transform) };
		shader->SetUniformBuffer(cb);
		auto target = RendererBase::GetmainRenderTargetView();
		RendererBase::Getd3dDeviceContext()->OMSetRenderTargets(1, &(target), RendererBase::GetDepthStencilView());
		RendererBase::DrawIndexed(*buffer);
	}
	void Renderer::Submit(Mesh* mesh, Shader* shader, DirectX::XMMATRIX transform)
	{
		Buffer buffer = { mesh->GetVertexBuffer(), mesh->GetIndexBuffer()};
		shader->Bind(ShaderType::Vertex);
		shader->Bind(ShaderType::Pixel);
		ConstantBuffer cb;
		cb = { m_viewProjectionMatrix, DirectX::XMMatrixTranspose(transform) };
		shader->SetUniformBuffer(cb);
		auto target = RendererBase::GetmainRenderTargetView();
		RendererBase::Getd3dDeviceContext()->OMSetRenderTargets(1, &(target), RendererBase::GetDepthStencilView());
		RendererBase::DrawIndexed(buffer);
	}
	
}