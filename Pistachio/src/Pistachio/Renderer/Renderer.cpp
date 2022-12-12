#include "ptpch.h"
#include "Renderer.h"
#include "Pistachio/Core/Window.h"
Pistachio::CamData Pistachio::Renderer::m_camera = { 0 };
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
		if (m_camera.cameratype == CameraType::Perspective)
			cb = { ((PerspectiveCamera*)m_camera.camera)->GetViewProjectionMatrix(), DirectX::XMMatrixTranspose(transform) };
		else
			cb = { ((PerspectiveCamera*)m_camera.camera)->GetViewProjectionMatrix(), DirectX::XMMatrixTranspose(transform) };
		shader->SetUniformBuffer(cb);
		auto target = RendererBase::GetmainRenderTargetView();
		RendererBase::Getd3dDeviceContext()->OMSetRenderTargets(1, &(target), RendererBase::GetDepthStencilView());
		RendererBase::DrawIndexed(*buffer);
	}
}