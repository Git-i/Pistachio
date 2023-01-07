#include "ptpch.h"
#include "Renderer.h"
#include "Renderer2D.h"
#include "Pistachio/Core/Window.h"
#include "DirectX11/DX11Texture.h"
Pistachio::PerspectiveCamera* Pistachio::Renderer::m_camera = nullptr;
Pistachio::RenderCubeMap Pistachio::Renderer::fbo = Pistachio::RenderCubeMap();
Pistachio::RenderCubeMap Pistachio::Renderer::ifbo = Pistachio::RenderCubeMap();
Pistachio::RenderCubeMap Pistachio::Renderer::prefilter = { Pistachio::RenderCubeMap() };
ID3D11ShaderResourceView* Pistachio::Renderer::pBrdfSRV = nullptr;
ID3D11RenderTargetView* Pistachio::Renderer::pBrdfRTV = nullptr;
ID3D11RenderTargetView* Pistachio::Renderer::m_target[8] = {0};
ID3D11DepthStencilView* Pistachio::Renderer::m_pDSV = nullptr;
int Pistachio::Renderer::m_NumRenderTextures = 1;
namespace Pistachio {
	void Renderer::Init(const char* skybox)
	{
		ID3D11ShaderResourceView* nullsrv[1] = { nullptr };
		DX11Texture::Bind(nullsrv);
		DX11Texture::Bind(nullsrv,1);
		DX11Texture::Bind(nullsrv,2);
		DX11Texture::Bind(nullsrv,3);
		if (pBrdfSRV)
		{
			pBrdfSRV->Release();
			pBrdfRTV->Release();
			fbo.GetSRV()->Release();
			fbo.GetRenderTexture()->Release();
			for (int i = 0; i < 6; i++)
				fbo.GetRTV(i)->Release();
			ifbo.GetSRV()->Release();
			ifbo.GetRenderTexture()->Release();
			for (int i = 0; i < 6; i++)
				ifbo.GetRTV(i)->Release();
			prefilter.GetSRV()->Release();
			prefilter.GetRenderTexture()->Release();
			for (int i = 0; i < 6; i++)
				prefilter.GetRTV(i)->Release();
		}
		RendererBase::SetCullMode(CullMode::Front);
		SamplerState* ss = SamplerState::Create(TextureAddress::Wrap, TextureAddress::Wrap, TextureAddress::Wrap);
		ss->Bind();
		FloatTexture2D tex;
		tex.CreateStack(skybox);
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
		fbo.CreateStack(512, 512, 6);
		float clearcolor[] = { 0.7f, 0.7f, 0.7f, 1.0f };
		DirectX::XMMATRIX captureProjection = DirectX::XMMatrixPerspectiveFovLH(DirectX::XMConvertToRadians(90.0f), 1.0f, 0.1f, 10.0f);
		DirectX::XMMATRIX captureViews[] =
		{
			DirectX::XMMatrixLookAtLH(DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f), DirectX::XMVectorSet( 1.0f,  0.0f,  0.0f, 1.0f), DirectX::XMVectorSet(0.0f, -1.0f,  0.0f, 1.0f)),
			DirectX::XMMatrixLookAtLH(DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f), DirectX::XMVectorSet(-1.0f,  0.0f,  0.0f, 1.0f), DirectX::XMVectorSet(0.0f, -1.0f,  0.0f, 1.0f)),
			DirectX::XMMatrixLookAtLH(DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f), DirectX::XMVectorSet( 0.0f, -1.0f,  0.0f, 1.0f), DirectX::XMVectorSet(0.0f,  0.0f,  1.0f, 1.0f)),
			DirectX::XMMatrixLookAtLH(DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f), DirectX::XMVectorSet( 0.0f,  1.0f,  0.0f, 1.0f), DirectX::XMVectorSet(0.0f,  0.0f, -1.0f, 1.0f)),
			DirectX::XMMatrixLookAtLH(DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f), DirectX::XMVectorSet( 0.0f,  0.0f, -1.0f, 1.0f), DirectX::XMVectorSet(0.0f, -1.0f,  0.0f, 1.0f)),
			DirectX::XMMatrixLookAtLH(DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f), DirectX::XMVectorSet( 0.0f,  0.0f,  1.0f, 1.0f), DirectX::XMVectorSet(0.0f, -1.0f,  0.0f, 1.0f))
		};
		RendererBase::ChangeViewport(512, 512);
		eqShader.Bind(ShaderType::Vertex);
		eqShader.Bind(ShaderType::Pixel);
		tex.Bind(0);
		for (int i = 0; i < 6; i++) {
			fbo.Bind(i);
			fbo.Clear(clearcolor, i);
			Buffer buffer = { cube.GetVertexBuffer(), cube.GetIndexBuffer() };
			
			ConstantBuffer cb;
			DirectX::XMFLOAT3 campos = { 0.0, 0.0, 0.0 };
			cb = { DirectX::XMMatrixTranspose(captureViews[i] * captureProjection), DirectX::XMMatrixTranspose(DirectX::XMMatrixIdentity()) , DirectX::XMLoadFloat3(&campos),{0.0, 0.0, 0.0},0 ,0 ,0};
			eqShader.SetVSRandomBuffer(&cb, sizeof(cb));
			RendererBase::DrawIndexed(buffer);
		}
		RendererBase::Getd3dDeviceContext()->GenerateMips(fbo.GetSRV());
		ifbo.CreateStack(32, 32);
		RendererBase::ChangeViewport(32, 32);
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
			irradianceShader.SetVSRandomBuffer(&cb, sizeof(cb));
			fbo.BindResource(1);
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
		RenderTexture texture[6] = {};
		prefilter.CreateStack(128, 128, 5);
		for (unsigned int mip = 0; mip < maxMipLevels; ++mip)
		{
			unsigned int mipWidth = 128 * std::pow(0.5, mip);
			unsigned int mipHeight = 128 * std::pow(0.5, mip);
			for (int i = 0; i < 6; ++i)
			{
				texture[i].CreateStack(mipWidth, mipHeight, 1, TextureFormat::RGBA16F);
			}
			// reisze framebuffer according to mip-level size.
			RendererBase::ChangeViewport(mipWidth, mipHeight);
			for (unsigned int i = 0; i < 6; ++i)
			{
				Buffer buffer = { cube.GetVertexBuffer(), cube.GetIndexBuffer() };
				PrefilterShaderConstBuffer.viewproj = DirectX::XMMatrixTranspose(captureViews[i] * captureProjection);
				PrefilterShaderConstBuffer.transform = DirectX::XMMatrixTranspose(DirectX::XMMatrixIdentity());
				PrefilterShaderConstBuffer.roughness = (float)mip / (float)(maxMipLevels - 1);
				prefilterShader.SetVSRandomBuffer(&PrefilterShaderConstBuffer, sizeof(PrefilterShaderConstBuffer));
				texture[i].Bind();
				texture[i].Clear(clearcolor);
				fbo.BindResource(1);
				RendererBase::DrawIndexed(buffer);
			}
			D3D11_BOX sourceRegion;
			for (int i = 0; i < 6; ++i)
			{
				sourceRegion.left = 0;
				sourceRegion.right = mipWidth;
				sourceRegion.top = 0;
				sourceRegion.bottom = mipHeight;
				sourceRegion.front = 0;
				sourceRegion.back = 1;
		
				RendererBase::Getd3dDeviceContext()->CopySubresourceRegion(prefilter.GetRenderTexture(), D3D11CalcSubresource(mip, i, 5), 0, 0, 0, texture[i].GetRenderTexture(), 0, &sourceRegion);
			}
			for (int i = 0; i < 6; ++i)
			{
				texture[i].Shutdown();
			}
		}
		for (int i = 0; i < 6; ++i)
		{
			texture[i].Shutdown();
		}
		ID3D11Texture2D* BrdfLUT = NULL;
		
		D3D11_TEXTURE2D_DESC textureDesc;
		D3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc;
		D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc;

		ZeroMemory(&textureDesc, sizeof(textureDesc));
		
		textureDesc.Width = 512;
		textureDesc.Height = 512;
		textureDesc.MipLevels = 1;
		textureDesc.ArraySize = 1;
		textureDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
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
		RendererBase::ChangeViewport( 512, 512);
		BrdfLUT->Release();
		BrdfLUT = NULL;
		brdfShader.Bind(ShaderType::Vertex);
		brdfShader.Bind(ShaderType::Pixel);
		RendererBase::Getd3dDeviceContext()->OMSetRenderTargets(1, &pBrdfRTV, nullptr);
		Buffer buffer = { plane.GetVertexBuffer(), plane.GetIndexBuffer() };
		RendererBase::DrawIndexed(buffer);
		auto data = GetWindowDataPtr();
		RendererBase::ChangeViewport(((WindowData*)(data))->width, ((WindowData*)(data))->height);
		RendererBase::SetCullMode(CullMode::Back);
		auto mainTarget = RendererBase::GetmainRenderTargetView();
		RendererBase::Getd3dDeviceContext()->OMSetRenderTargets(1, &mainTarget, RendererBase::GetDepthStencilView());
		Pistachio::DX11Texture::Bind(&pBrdfSRV, 0);
		ifbo.BindResource(1);
		prefilter.BindResource(2);
		fbo.BindResource(3);
		cube.DestroyMesh();
		plane.DestroyMesh();
		delete ss;
	}
	void Renderer::EndScene()
	{
		auto data = ((WindowData*)GetWindowDataPtr());
		if (data->vsync)
			RendererBase::GetSwapChain()->Present(1, 0);
		else
			RendererBase::GetSwapChain()->Present(0, DXGI_PRESENT_DO_NOT_WAIT | DXGI_PRESENT_ALLOW_TEARING);
	}
	void Renderer::Shutdown() {
		if (pBrdfRTV) {
			while (pBrdfRTV->Release()) {};
			pBrdfRTV = NULL;
		}
		if (pBrdfSRV) {
			while (pBrdfSRV->Release()) {};
			pBrdfSRV = NULL;
		}
		
		fbo.ShutDown();
		ifbo.ShutDown();
		prefilter.ShutDown();
		Renderer2D::Shutdown();
		RendererBase::Shutdown();
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
	void Renderer::Submit(Mesh* mesh, Shader* shader, float* c, float m, float r, float ao, const DirectX::XMMATRIX& transform, const DirectX::XMMATRIX& viewProj)
	{
		Buffer buffer = { mesh->GetVertexBuffer(), mesh->GetIndexBuffer()};
		shader->Bind(ShaderType::Vertex);
		shader->Bind(ShaderType::Pixel);
		ConstantBuffer cb;
		auto campos = m_camera->GetPosition();
		cb = { viewProj, DirectX::XMMatrixTranspose(transform) , DirectX::XMLoadFloat3(&campos),{c[0], c[1], c[2]},m,r,ao};
		shader->SetUniformBuffer(cb);
		auto target = RendererBase::GetmainRenderTargetView();
		if (m_target)
			RendererBase::Getd3dDeviceContext()->OMSetRenderTargets(m_NumRenderTextures, m_target, m_pDSV);
		else
			RendererBase::Getd3dDeviceContext()->OMSetRenderTargets(1, &(target), RendererBase::GetDepthStencilView());
		RendererBase::DrawIndexed(buffer);
	}
	
}