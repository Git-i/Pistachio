#include "ptpch.h"
#include "Renderer.h"
#include "Pistachio/Core/Window.h"
#include "DirectX11/DX11Texture.h"
Pistachio::PerspectiveCamera* Pistachio::Renderer::m_camera = nullptr;
Pistachio::RenderCubeMap Pistachio::Renderer::fbo = Pistachio::RenderCubeMap();
Pistachio::RenderCubeMap Pistachio::Renderer::ifbo = Pistachio::RenderCubeMap();
Pistachio::RenderCubeMap Pistachio::Renderer::prefilter[5] = { Pistachio::RenderCubeMap	() };
ID3D11ShaderResourceView* Pistachio::Renderer::pBrdfSRV = nullptr;
ID3D11RenderTargetView* Pistachio::Renderer::pBrdfRTV = nullptr;
namespace Pistachio {
	void Renderer::Init()
	{
		SamplerState* ss = SamplerState::Create();
		ss->Bind();
		Texture3D* tex = Texture3D::Create("resources/textures/hdr/newport_loft.hdr");
		Shader eqShader(L"equirectangular_to_cubemap_vs.cso", L"equirectangular_to_cubemap_fs.cso");
		Shader irradianceShader(L"equirectangular_to_cubemap_vs.cso", L"irradiance_fs.cso");
		Shader brdfShader(L"brdf_vs.cso", L"brdf_fs.cso");
		Shader prefilterShader(L"prefilter_vs.cso", L"prefilter_fs.cso");
		eqShader.CreateLayout(Mesh::GetLayout(), Mesh::GetLayoutSize());
		irradianceShader.CreateLayout(Mesh::GetLayout(), Mesh::GetLayoutSize());
		prefilterShader.CreateLayout(Mesh::GetLayout(), Mesh::GetLayoutSize());
		brdfShader.CreateLayout(Mesh::GetLayout(), Mesh::GetLayoutSize());
		Mesh cube;
		Mesh plane;
		cube.CreateStack("cube.obj");
		plane.CreateStack("plane.obj");
		fbo.CreateStack(512, 512,5);
		float clearcolor[] = { 0.7f, 0.7f, 0.7f, 1.0f };
		DirectX::XMMATRIX captureProjection = DirectX::XMMatrixPerspectiveFovLH(DirectX::XMConvertToRadians(90.0f), 1.0f, 0.1f, 10.0f);
		DirectX::XMMATRIX captureViews[] =
		{
			DirectX::XMMatrixLookAtLH(DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f), DirectX::XMVectorSet(1.0f, 0.0f, 0.0f , 1.0f), DirectX::XMVectorSet(0.0f, -1.0f, 0.0f  ,1.0f)),
			DirectX::XMMatrixLookAtLH(DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f), DirectX::XMVectorSet(-1.0f, 0.0f, 0.0f , 1.0f), DirectX::XMVectorSet(0.0f, -1.0f,  0.0f,1.0f)),
			DirectX::XMMatrixLookAtLH(DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f), DirectX::XMVectorSet(0.0f, -1.0f, 0.0f , 1.0f), DirectX::XMVectorSet(0.0f,  0.0f,  1.0f,1.0f)),
			DirectX::XMMatrixLookAtLH(DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f), DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 1.0f), DirectX::XMVectorSet(0.0f,  0.0f, -1.0f,1.0f)),
			DirectX::XMMatrixLookAtLH(DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f), DirectX::XMVectorSet(0.0f, 0.0f, -1.0f , 1.0f), DirectX::XMVectorSet(0.0f, -1.0f,  0.0f,1.0f)),
			DirectX::XMMatrixLookAtLH(DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f), DirectX::XMVectorSet(0.0f, 0.0f, 1.0f, 1.0f), DirectX::XMVectorSet(0.0f, -1.0f,  0.0f ,1.0f))
		};
		RendererBase::Resize(512, 512);
		eqShader.Bind(ShaderType::Vertex);
		eqShader.Bind(ShaderType::Pixel);
		for (int i = 0; i < 6; i++) {
			fbo.Bind(i);
			fbo.Clear(clearcolor, i);
			Buffer buffer = { cube.GetVertexBuffer(), cube.GetIndexBuffer() };
			
			ConstantBuffer cb;
			DirectX::XMFLOAT3 campos = { 0.0, 0.0, 0.0 };
			cb = { DirectX::XMMatrixTranspose(captureViews[i] * captureProjection), DirectX::XMMatrixTranspose(DirectX::XMMatrixIdentity()) , DirectX::XMLoadFloat3(&campos),{0.0, 0.0, 0.0},0 ,0 ,0};
			eqShader.SetRandomConstantBuffer(&cb, sizeof(cb));
			tex->Bind(0);
			RendererBase::DrawIndexed(buffer);
		}
		RendererBase::Getd3dDeviceContext()->GenerateMips(fbo.shaderResourceView);
		ifbo.CreateStack(32, 32);
		RendererBase::Resize(32, 32);
		irradianceShader.Bind(ShaderType::Vertex);
		irradianceShader.Bind(ShaderType::Pixel);
		for (int i = 0; i < 6; i++)
		{
			Buffer buffer = { cube.GetVertexBuffer(), cube.GetIndexBuffer() };
			ifbo.Bind(i);
			ifbo.Clear(clearcolor, i);
			
			ConstantBuffer cb;
			DirectX::XMFLOAT3 campos = { 0.0, 0.0, 0.0 };
			cb = { DirectX::XMMatrixTranspose(captureViews[i] * captureProjection), DirectX::XMMatrixTranspose(DirectX::XMMatrixIdentity()) , DirectX::XMLoadFloat3(&campos),{0.0, 0.0, 0.0},0 ,0 ,0 };
			irradianceShader.SetRandomConstantBuffer(&cb, sizeof(cb));
			DX11Texture::Bind(Renderer::GetFrambufferSRV(), 1);
			RendererBase::DrawIndexed(buffer);
		}
		struct{
			DirectX::XMMATRIX viewproj;
			DirectX::XMMATRIX transform;
			float roughness;
		}PrefilterShaderConstBuffer;
		prefilterShader.Bind(ShaderType::Vertex);
		prefilterShader.Bind(ShaderType::Pixel);
		unsigned int maxMipLevels = 5;
		for (unsigned int mip = 0; mip < maxMipLevels; ++mip)
		{
			// reisze framebuffer according to mip-level size.
			unsigned int mipWidth = 128 * std::pow(0.5, mip);
			unsigned int mipHeight = 128 * std::pow(0.5, mip);
			if (mipWidth != 8)
			prefilter[mip].CreateStack(mipWidth, mipHeight, 5);
			else
			prefilter[mip].CreateStack(mipWidth, mipHeight);
			RendererBase::Resize(mipWidth, mipHeight);
			PrefilterShaderConstBuffer.roughness = (float)mip / (float)(maxMipLevels - 1);
			for (unsigned int i = 0; i < 6; ++i)
			{
				Buffer buffer = { cube.GetVertexBuffer(), cube.GetIndexBuffer() };
				PrefilterShaderConstBuffer.viewproj = DirectX::XMMatrixTranspose(captureViews[i] * captureProjection);
				PrefilterShaderConstBuffer.transform = DirectX::XMMatrixTranspose(DirectX::XMMatrixIdentity());
				prefilterShader.SetRandomConstantBuffer(&PrefilterShaderConstBuffer, sizeof(PrefilterShaderConstBuffer));
				prefilter[mip].Bind(i);
				prefilter[mip].Clear(clearcolor, i);
				DX11Texture::Bind(Renderer::GetFrambufferSRV(), 1);
				RendererBase::DrawIndexed(buffer);
			}
		}
		RendererBase::Getd3dDeviceContext()->GenerateMips(prefilter[0].shaderResourceView);
		ID3D11Texture2D* BrdfLUT;
		
		D3D11_TEXTURE2D_DESC textureDesc;
		D3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc;
		D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc;

		ZeroMemory(&textureDesc, sizeof(textureDesc));
		
		textureDesc.Width = 512;
		textureDesc.Height = 512;
		textureDesc.MipLevels = 1;
		textureDesc.ArraySize = 1;
		textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		textureDesc.SampleDesc.Count = 1;
		textureDesc.SampleDesc.Quality = 0;
		textureDesc.Usage = D3D11_USAGE_DEFAULT;
		textureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
		textureDesc.CPUAccessFlags = 0;
		textureDesc.MiscFlags = 0;

		RendererBase::Getd3dDevice()->CreateTexture2D(&textureDesc, NULL, &BrdfLUT);

		shaderResourceViewDesc.Format = textureDesc.Format;
		shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		shaderResourceViewDesc.Texture2D.MostDetailedMip = 0;
		shaderResourceViewDesc.Texture2D.MipLevels = 1;

		
		renderTargetViewDesc.Format = textureDesc.Format;
		renderTargetViewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
		renderTargetViewDesc.Texture2DArray.MipSlice = 0;
		renderTargetViewDesc.Texture2DArray.ArraySize = 1;
		RendererBase::Getd3dDevice()->CreateRenderTargetView(BrdfLUT, &renderTargetViewDesc, &pBrdfRTV);
		
		RendererBase::Getd3dDevice()->CreateShaderResourceView(BrdfLUT, &shaderResourceViewDesc, &pBrdfSRV);
		RendererBase::Resize( 512, 512);
		brdfShader.Bind(ShaderType::Vertex);
		brdfShader.Bind(ShaderType::Pixel);
		RendererBase::Getd3dDeviceContext()->OMSetRenderTargets(1, &pBrdfRTV, RendererBase::GetDepthStencilView());
		Buffer buffer = { plane.GetVertexBuffer(), plane.GetIndexBuffer() };
		RendererBase::DrawIndexed(buffer);



		auto target = RendererBase::GetmainRenderTargetView();
		RendererBase::Getd3dDeviceContext()->OMSetRenderTargets(1, &target, RendererBase::GetDepthStencilView());
		RendererBase::DrawIndexed(buffer);
		RendererBase::Resize(1280, 720);
	}
	void Renderer::EndScene()
	{
		auto data = ((WindowData*)GetWindowDataPtr());
		RendererBase::GetSwapChain()->Present(data->vsync, DXGI_PRESENT_ALLOW_TEARING);
	}
	void Renderer::Submit(Buffer* buffer, Shader* shader, DirectX::XMMATRIX transform)
	{
		shader->Bind(ShaderType::Vertex);
		shader->Bind(ShaderType::Pixel);
		ConstantBuffer cb;
		auto campos = m_camera->GetPosition();
		cb = { m_camera->GetViewProjectionMatrix(), DirectX::XMMatrixTranspose(transform) , DirectX::XMLoadFloat3(&campos) };
		shader->SetUniformBuffer(cb);
		auto target = RendererBase::GetmainRenderTargetView();
		RendererBase::Getd3dDeviceContext()->OMSetRenderTargets(1, &(target), RendererBase::GetDepthStencilView());
		RendererBase::DrawIndexed(*buffer);
	}
	void Renderer::Submit(Mesh* mesh, Shader* shader, float* c, float m, float r, float ao, bool a, DirectX::XMMATRIX transform, DirectX::XMMATRIX viewProj)
	{
		Buffer buffer = { mesh->GetVertexBuffer(), mesh->GetIndexBuffer()};
		shader->Bind(ShaderType::Vertex);
		shader->Bind(ShaderType::Pixel);
		ConstantBuffer cb;
		auto campos = m_camera->GetPosition();
		cb = { viewProj, DirectX::XMMatrixTranspose(transform) , DirectX::XMLoadFloat3(&campos),{c[0], c[1], c[2]},m,r,ao};
		shader->SetUniformBuffer(cb);
		auto target = RendererBase::GetmainRenderTargetView();
		if(a)
		RendererBase::Getd3dDeviceContext()->OMSetRenderTargets(1, &(target), RendererBase::GetDepthStencilView());
		RendererBase::DrawIndexed(buffer);
	}
	
}