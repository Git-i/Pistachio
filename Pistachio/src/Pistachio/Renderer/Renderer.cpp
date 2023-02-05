#include "ptpch.h"
#include "Renderer.h"
#include "Renderer2D.h"
#include "Pistachio/Core/Window.h"
#include "DirectX11/DX11Texture.h"
DirectX::XMMATRIX Pistachio::Renderer::viewproj = DirectX::XMMatrixIdentity();
DirectX::XMVECTOR Pistachio::Renderer::m_campos = DirectX::XMVectorZero();
ID3D11ShaderResourceView* Pistachio::Renderer::pBrdfSRV = nullptr;
ID3D11RenderTargetView* Pistachio::Renderer::pBrdfRTV = nullptr;
Pistachio::ShaderLibrary Pistachio::Renderer::shaderlib = Pistachio::ShaderLibrary();
Pistachio::ConstantBuffer Pistachio::Renderer::MaterialCB = Pistachio::ConstantBuffer();
Pistachio::ConstantBuffer Pistachio::Renderer::LightCB = Pistachio::ConstantBuffer();
Pistachio::ConstantBuffer Pistachio::Renderer::ShadowCB = Pistachio::ConstantBuffer();
Pistachio::ConstantBuffer Pistachio::Renderer::TransformationBuffer = Pistachio::ConstantBuffer();
Pistachio::ConstantBuffer Pistachio::Renderer::CameraCB = {};
Pistachio::RenderCubeMap Pistachio::Renderer::fbo = Pistachio::RenderCubeMap();
Pistachio::RenderCubeMap Pistachio::Renderer::ifbo = Pistachio::RenderCubeMap();
Pistachio::RenderCubeMap Pistachio::Renderer::prefilter = { Pistachio::RenderCubeMap() };
Pistachio::Renderer::LD  Pistachio::Renderer::LightData = {};
Pistachio::Renderer::ShadowData Pistachio::Renderer::shadowData = {};
Pistachio::Light* Pistachio::Renderer::lightIndexPtr = nullptr;
Pistachio::Renderer::CamerData Pistachio::Renderer::CameraData = {};
Pistachio::Texture2D Pistachio::Renderer::whiteTexture = Pistachio::Texture2D();
namespace Pistachio {
	void Renderer::Init(const char* skybox)
	{
		PT_PROFILE_FUNCTION();
		MaterialStruct cb;
		struct CameraStruct {
			DirectX::XMMATRIX viewproj;
			DirectX::XMFLOAT4 viewPos;
		} camerabufferData;
		MaterialCB.Create(&cb, sizeof(MaterialStruct));
		LightCB.Create(&LightData, sizeof(LightData));
		CameraCB.Create(&camerabufferData, sizeof(camerabufferData));
		ShadowCB.Create(&shadowData, sizeof(shadowData));
		SamplerState* brdfsampler = SamplerState::Create(TextureAddress::Clamp, TextureAddress::Clamp, TextureAddress::Clamp);
		SamplerState* shadowsampler = SamplerState::Create(TextureAddress::Border, TextureAddress::Border, TextureAddress::Border);
		brdfsampler->Bind(1);
		shadowsampler->Bind(2);
		TransformationBuffer.Create(&camerabufferData, sizeof(DirectX::XMMATRIX)*2);
		if (pBrdfSRV)
		{
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
		Shader eqShader(L"resources/shaders/vertex/equirectangular_to_cubemap_vs.cso", L"resources/shaders/pixel/equirectangular_to_cubemap_fs.cso");
		Shader irradianceShader(L"resources/shaders/vertex/equirectangular_to_cubemap_vs.cso", L"resources/shaders/pixel/irradiance_fs.cso");
		Shader brdfShader(L"resources/shaders/vertex/brdf_vs.cso", L"resources/shaders/pixel/brdf_fs.cso");
		Shader prefilterShader(L"resources/shaders/vertex/prefilter_vs.cso", L"resources/shaders/pixel/prefilter_fs.cso");
		eqShader.CreateLayout(Mesh::GetLayout(), Mesh::GetLayoutSize());
		irradianceShader.CreateLayout(Mesh::GetLayout(), Mesh::GetLayoutSize());
		prefilterShader.CreateLayout(Mesh::GetLayout(), Mesh::GetLayoutSize());
		brdfShader.CreateLayout(Mesh::GetLayout(), Mesh::GetLayoutSize());
		Mesh cube;
		Mesh plane;
		cube.CreateStack("cube.obj");
		plane.CreateStack("plane.obj");
		auto& cubeVB = cube.GetVertexBuffer();
		auto& cubeIB = cube.GetIndexBuffer();
		auto& planeVB = plane.GetVertexBuffer();
		auto& planeIB = plane.GetIndexBuffer();
		Buffer buffer = { &cubeVB, &cubeIB };
		Buffer planebuffer = { &planeVB, &planeIB };
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
			
			
			DirectX::XMFLOAT4 campos = { 0.0, 0.0, 0.0, 1.0 };
			camerabufferData = { DirectX::XMMatrixMultiplyTranspose(captureViews[i], captureProjection), campos};
			CameraCB.Update(&camerabufferData, sizeof(DirectX::XMMATRIX));
			eqShader.SetVSBuffer(CameraCB, 0);
			RendererBase::DrawIndexed(buffer);
		}
		RendererBase::Getd3dDeviceContext()->GenerateMips(fbo.GetSRV());
		ifbo.CreateStack(32, 32);
		RendererBase::ChangeViewport(32, 32);
		irradianceShader.Bind(ShaderType::Vertex);
		irradianceShader.Bind(ShaderType::Pixel);
		for (int i = 0; i < 6; i++)
		{
			
			ifbo.Bind(i);
			ifbo.Clear(clearcolor, i);
			

			DirectX::XMFLOAT4 campos = { 0.0, 0.0, 0.0, 1.0 };
			camerabufferData = { DirectX::XMMatrixMultiplyTranspose(captureViews[i], captureProjection), campos };
			CameraCB.Update(&camerabufferData, sizeof(DirectX::XMMATRIX));
			irradianceShader.SetVSBuffer(CameraCB, 0);
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
		RenderTextureDesc desc;
		desc.miplevels = 1;
		desc.Attachments = { TextureFormat::RGBA16F };
		ConstantBuffer pfCB;
		pfCB.Create(&PrefilterShaderConstBuffer, sizeof(PrefilterShaderConstBuffer));
		for (unsigned int mip = 0; mip < maxMipLevels; ++mip)
		{
			unsigned int mipWidth = (int)(128 * std::pow(0.5, mip));
			unsigned int mipHeight = (int)(128 * std::pow(0.5, mip));
			for (int i = 0; i < 6; ++i)
			{
				desc.width = mipWidth;
				desc.height = mipHeight;
				texture[i].CreateStack(desc);
			}
			// reisze framebuffer according to mip-level size.
			RendererBase::ChangeViewport(mipWidth, mipHeight);
			for (unsigned int i = 0; i < 6; ++i)
			{
				
				PrefilterShaderConstBuffer.viewproj = DirectX::XMMatrixTranspose(captureViews[i] * captureProjection);
				PrefilterShaderConstBuffer.transform = DirectX::XMMatrixTranspose(DirectX::XMMatrixIdentity());
				PrefilterShaderConstBuffer.roughness = (float)mip / (float)(maxMipLevels - 1);
				pfCB.Update(&PrefilterShaderConstBuffer, sizeof(PrefilterShaderConstBuffer));
				prefilterShader.SetVSBuffer(pfCB, 0);
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
				ID3D11Resource* pTexture;
				texture[i].GetRenderTexture(&pTexture);
				RendererBase::Getd3dDeviceContext()->CopySubresourceRegion(prefilter.GetRenderTexture(), D3D11CalcSubresource(mip, i, 5), 0, 0, 0, pTexture, 0, &sourceRegion);
				pTexture->Release();
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
		D3D11_TEXTURE2D_DESC textureDesc;
		D3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc;
		D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc;
		if (!pBrdfSRV) {
			ID3D11Texture2D* BrdfLUT = NULL;


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
			RendererBase::ChangeViewport(512, 512);
			BrdfLUT->Release();
			BrdfLUT = NULL;
			brdfShader.Bind(ShaderType::Vertex);
			brdfShader.Bind(ShaderType::Pixel);
			Pistachio::RendererBase::SetCullMode(CullMode::Front);
			RendererBase::Getd3dDeviceContext()->OMSetRenderTargets(1, &pBrdfRTV, nullptr);
			
			RendererBase::DrawIndexed(planebuffer);
			Ref<Shader> pbrShader = std::make_shared<Shader>(L"resources/shaders/vertex/VertexShader.cso", L"resources/shaders/pixel/PixelShader.cso");
			pbrShader->CreateLayout(Mesh::GetLayout(), Mesh::GetLayoutSize());
			Ref<Shader> shadowShader = std::make_shared<Shader>(L"resources/shaders/vertex/VertexShader.cso", L"resources/shaders/pixel/Shadow_ps.cso");
			shadowShader->CreateLayout(Mesh::GetLayout(), Mesh::GetLayoutSize());
			shaderlib.Add("PBR-Shader", pbrShader);
			shaderlib.Add("Shadow-Shader", shadowShader);

			char data[4] = { 255,255,255,255 };
			whiteTexture.CreateStack(1, 1, TextureFormat::RGBA8U,data);
		}
		Shader::SetPSBuffer(LightCB, 0);
		Shader::SetVSBuffer(CameraCB, 0);
		Shader::SetPSBuffer(MaterialCB, 1);
		Shader::SetVSBuffer(TransformationBuffer, 1);
		Shader::SetVSBuffer(ShadowCB, 2);
		auto data = GetWindowDataPtr();
		RendererBase::ChangeViewport(((WindowData*)(data))->width, ((WindowData*)(data))->height);
		RendererBase::SetCullMode(CullMode::Back);
		auto mainTarget = RendererBase::GetmainRenderTargetView();
		RendererBase::Getd3dDeviceContext()->OMSetRenderTargets(1, &mainTarget, RendererBase::GetDepthStencilView());
		cube.DestroyMesh();
		plane.DestroyMesh();
		delete ss;

		ZeroMemory(&LightData, sizeof(LightData));
		lightIndexPtr = LightData.lights;

	}
	void Renderer::AddLight(const Light& light)
	{
		*lightIndexPtr = light;
		lightIndexPtr++;
		shadowData.numlights.x++;
	}
	void Renderer::BeginScene(PerspectiveCamera* cam) {
		viewproj = cam->GetViewProjectionMatrix();
		auto campos = cam->GetPosition();
		Pistachio::DX11Texture::Bind(&pBrdfSRV, 0);
		ifbo.BindResource(1);
		prefilter.BindResource(2);
		CameraData.viewProjection = viewproj;
		CameraData.viewPos = {campos.x,campos.y, campos.z, 1.f};
		LightCB.Update(&LightData, sizeof(LightData));
		CameraCB.Update(&CameraData, sizeof(CameraData));
		
		whiteTexture.Bind(3);
		whiteTexture.Bind(4);
		whiteTexture.Bind(5);
	}
	void Renderer::BeginScene(RuntimeCamera* cam, const DirectX::XMMATRIX& transform) {
		DirectX::XMFLOAT4 campos;
		DirectX::XMStoreFloat4(&campos, transform.r[3]);
		viewproj = DirectX::XMMatrixMultiplyTranspose(DirectX::XMMatrixInverse(nullptr, transform), cam->GetProjection());
		Pistachio::DX11Texture::Bind(&pBrdfSRV, 0);
		ifbo.BindResource(1);
		prefilter.BindResource(2);
		CameraData.viewProjection = viewproj;
		CameraData.viewPos = campos;
		LightCB.Update(&LightData, sizeof(LightData));
		CameraCB.Update(&CameraData, sizeof(CameraData));
		Shader::SetPSBuffer(LightCB, 0);
		Shader::SetVSBuffer(CameraCB, 0);
		whiteTexture.Bind(3);
		whiteTexture.Bind(4);
		whiteTexture.Bind(5);
	}
	void Renderer::BeginScene(EditorCamera& cam)
	{
		DirectX::XMFLOAT4 campos;
		DirectX::XMStoreFloat4(&campos, cam.GetPosition());
		viewproj = DirectX::XMMatrixTranspose(cam.GetViewProjection());
		auto maintarget = RendererBase::GetmainRenderTargetView();
		Pistachio::DX11Texture::Bind(&pBrdfSRV, 0);
		ifbo.BindResource(1);
		prefilter.BindResource(2);
		CameraData.viewProjection = viewproj;
		CameraData.viewPos = campos;
		LightCB.Update(&LightData, sizeof(LightData));
		CameraCB.Update(&CameraData, sizeof(CameraData));
		ShadowCB.Update(&shadowData, sizeof(shadowData));
		//whiteTexture.Bind(3);
		whiteTexture.Bind(4);
		whiteTexture.Bind(5);
	}
	void Renderer::EndScene()
	{
		PT_PROFILE_FUNCTION()
		auto data = ((WindowData*)GetWindowDataPtr());
		if (data->vsync)
			RendererBase::GetSwapChain()->Present(1, 0);
		else
			RendererBase::GetSwapChain()->Present(0, DXGI_PRESENT_DO_NOT_WAIT | DXGI_PRESENT_ALLOW_TEARING);
		ZeroMemory(&LightData, sizeof(LightData));
		lightIndexPtr = LightData.lights;
		shadowData.numlights = {0,0,0,0};
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
	void Renderer::Submit(Mesh* mesh, Shader* shader, float* c, float m, float r, int ID, const DirectX::XMMATRIX& transform, const DirectX::XMMATRIX& viewProjection)
	{
		PT_PROFILE_FUNCTION();
		auto& VB = mesh->GetVertexBuffer(); 
		auto& IB = mesh->GetIndexBuffer();
		Buffer buffer = { &VB,&IB };
		MaterialStruct cb;
		cb = { {c[0], c[1], c[2], c[3]},m,r,ID};
		struct TransformData { DirectX::XMMATRIX transform; DirectX::XMMATRIX normal; } td;
		td = { DirectX::XMMatrixTranspose(transform), DirectX::XMMatrixInverse(nullptr, transform) };
		MaterialCB.Update(&cb, sizeof(MaterialStruct));
		TransformationBuffer.Update(&td, sizeof(TransformData));
		shader->Bind(ShaderType::Vertex);
		shader->Bind(ShaderType::Pixel);
		RendererBase::DrawIndexed(buffer);
	}
	
}