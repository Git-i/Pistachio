#include "ptpch.h"
#include "Renderer.h"
#include "Pistachio/Core/Window.h"
DirectX::XMMATRIX Pistachio::Renderer::ViewProjectionMatrix = DirectX::XMMatrixIdentity();
namespace Pistachio {
	void Renderer::BeginScene(OrthographicCamera& cam)
	{
		ViewProjectionMatrix = cam.GetViewProjectionMatrix();
	}
	void Renderer::EndScene()
	{
		auto data = ((WindowData*)GetWindowDataPtr());
		RendererBase::GetSwapChain()->Present(data->vsync, 0);
	}
	void Renderer::Submit(Buffer& buffer, Shader& shader)
	{
		shader.Bind(ShaderType::Vertex);
		shader.Bind(ShaderType::Pixel);
		ConstantBuffer cb = {ViewProjectionMatrix};
		shader.SetUniformBuffer(cb);
		auto target = RendererBase::GetmainRenderTargetView();
		RendererBase::Getd3dDeviceContext()->OMSetRenderTargets(1, &(target), NULL);
		RendererBase::DrawIndexed(buffer);
	}
}