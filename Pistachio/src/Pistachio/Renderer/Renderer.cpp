#include "ptpch.h"
#include "Renderer.h"
#include "Renderer2D.h"
#include "Pistachio/Core/Window.h"
#include "DirectX11/DX11Texture.h"
DirectX::XMMATRIX Pistachio::Renderer::viewproj = DirectX::XMMatrixIdentity();
DirectX::XMVECTOR Pistachio::Renderer::m_campos = DirectX::XMVectorZero();
Pistachio::RenderTexture Pistachio::Renderer::BrdfTex;
Pistachio::ShaderLibrary Pistachio::Renderer::shaderlib = Pistachio::ShaderLibrary();
Pistachio::ConstantBuffer Pistachio::Renderer::MaterialCB = Pistachio::ConstantBuffer();
Pistachio::ConstantBuffer Pistachio::Renderer::LightCB = Pistachio::ConstantBuffer();
std::vector<Pistachio::ConstantBuffer> Pistachio::Renderer::TransformationBuffer;
Pistachio::PassConstants Pistachio::Renderer::passConstants;
Pistachio::ConstantBuffer Pistachio::Renderer::PassCB = {};
Pistachio::RenderCubeMap Pistachio::Renderer::fbo = Pistachio::RenderCubeMap();
Pistachio::RenderCubeMap Pistachio::Renderer::ifbo = Pistachio::RenderCubeMap();
Pistachio::RenderCubeMap Pistachio::Renderer::prefilter = { Pistachio::RenderCubeMap() };
Pistachio::Renderer::LD  Pistachio::Renderer::LightData = {};
Pistachio::Light* Pistachio::Renderer::lightIndexPtr = nullptr;
Pistachio::Renderer::CamerData Pistachio::Renderer::CameraData = {};
Pistachio::Texture2D Pistachio::Renderer::whiteTexture = Pistachio::Texture2D();
Pistachio::Material Pistachio::Renderer::DefaultMaterial = Pistachio::Material();

static Pistachio::SamplerState* brdfSampler ;
static Pistachio::SamplerState* shadowSampler;
namespace Pistachio {
	void Renderer::CreateConstantBuffers()
	{
		PT_PROFILE_FUNCTION();
		MaterialStruct cb;
		MaterialCB.Create(nullptr, sizeof(MaterialStruct));
		LightCB.Create(&LightData, sizeof(LightData));
		struct CameraStruct {
			DirectX::XMMATRIX viewproj;
			DirectX::XMMATRIX view;
			DirectX::XMFLOAT4 viewPos;
		} camerabufferData;
		PassCB.Create(nullptr, sizeof(PassConstants));
	}
	void Renderer::Init(const char* skybox)
	{
		PT_PROFILE_FUNCTION();
		
		CreateConstantBuffers();
		struct CameraStruct {
			DirectX::XMMATRIX viewproj;
			DirectX::XMMATRIX view;
			DirectX::XMFLOAT4 viewPos;
		} camerabufferData;
		ConstantBuffer CameraCB;
		CameraCB.Create(&camerabufferData, sizeof(camerabufferData));
		brdfSampler = SamplerState::Create(SamplerStateDesc::Default);
		SamplerStateDesc sDesc = SamplerStateDesc::Default;
		sDesc.AddressU = sDesc.AddressV = sDesc.AddressW = TextureAddress::Border;
		sDesc.ComparisonEnable = true;
		sDesc.func = ComparisonFunc::LessOrEqual;
		shadowSampler = SamplerState::Create(sDesc);
		brdfSampler->Bind(1);
		shadowSampler->Bind(2);

		RendererBase::SetCullMode(CullMode::Front);
		sDesc.AddressU = sDesc.AddressV = sDesc.AddressW = TextureAddress::Wrap;
		sDesc.ComparisonEnable = false;
		SamplerState* ss = SamplerState::Create(sDesc);
		ss->Bind();

		Texture2D tex;
		tex.CreateStack(skybox, TextureFormat::RGBA32F);

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
			camerabufferData = { DirectX::XMMatrixMultiplyTranspose(captureViews[i], captureProjection), DirectX::XMMatrixIdentity(), campos};
			CameraCB.Update(&camerabufferData, sizeof(camerabufferData));
			eqShader.SetVSBuffer(CameraCB, 0);
			RendererBase::DrawIndexed(buffer);
		}
		RendererBase::Getd3dDeviceContext()->GenerateMips((ID3D11ShaderResourceView*)fbo.GetID().ptr);
		ifbo.CreateStack(32, 32);
		RendererBase::ChangeViewport(32, 32);
		irradianceShader.Bind(ShaderType::Vertex);
		irradianceShader.Bind(ShaderType::Pixel);
		for (int i = 0; i < 6; i++)
		{
			
			ifbo.Bind(i);
			ifbo.Clear(clearcolor, i);
			

			DirectX::XMFLOAT4 campos = { 0.0, 0.0, 0.0, 1.0 };
			camerabufferData = { DirectX::XMMatrixMultiplyTranspose(captureViews[i], captureProjection), DirectX::XMMatrixIdentity(),campos};
			CameraCB.Update(&camerabufferData, sizeof(camerabufferData));
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
		RenderTexture texture = {};
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
				texture.CreateStack(desc);
			}
			// reisze framebuffer according to mip-level size.
			RendererBase::ChangeViewport(mipWidth, mipHeight);
			D3D11_BOX sourceRegion;
			for (unsigned int i = 0; i < 6; ++i)
			{
				
				PrefilterShaderConstBuffer.viewproj = DirectX::XMMatrixTranspose(captureViews[i] * captureProjection);
				PrefilterShaderConstBuffer.transform = DirectX::XMMatrixTranspose(DirectX::XMMatrixIdentity());
				PrefilterShaderConstBuffer.roughness = (float)mip / (float)(maxMipLevels - 1);
				pfCB.Update(&PrefilterShaderConstBuffer, sizeof(PrefilterShaderConstBuffer));
				prefilterShader.SetVSBuffer(pfCB, 0);
				texture.Bind();
				texture.Clear(clearcolor, 0);
				fbo.BindResource(1);
				RendererBase::DrawIndexed(buffer);

				sourceRegion.left = 0;
				sourceRegion.right = mipWidth;
				sourceRegion.top = 0;
				sourceRegion.bottom = mipHeight;
				sourceRegion.front = 0;
				sourceRegion.back = 1;
				Texture2D lmao = texture.GetRenderTexture(0);
				prefilter.GetResource().CopyIntoRegion(lmao, 0, 0, sourceRegion.left, sourceRegion.right, sourceRegion.top, sourceRegion.bottom, mip, i);
			}
		}
		Pistachio::RenderTextureDesc descBRDF;
		descBRDF.Attachments = { TextureFormat::RGBA16F };
		descBRDF.height = 512;
		descBRDF.width = 512;
		BrdfTex.CreateStack(descBRDF);
		brdfShader.Bind(ShaderType::Vertex);
		brdfShader.Bind(ShaderType::Pixel);
		Pistachio::RendererBase::SetCullMode(CullMode::Front);
		BrdfTex.Bind();
		RendererBase::DrawIndexed(planebuffer);

		Ref<Shader> pbrShader = std::make_shared<Shader>(L"resources/shaders/vertex/VertexShader.cso", L"resources/shaders/pixel/PixelShader.cso");
		pbrShader->CreateLayout(Mesh::GetLayout(), Mesh::GetLayoutSize());
		Ref<Shader> shadowShader = std::make_shared<Shader>(L"resources/shaders/vertex/shadow_vs.cso", L"resources/shaders/pixel/Shadow_ps.cso", L"resources/shaders/geometry/shadow_gs.cso");
		shadowShader->CreateLayout(Mesh::GetLayout(), Mesh::GetLayoutSize());
		Ref<Shader> gBuffer_Shader = std::make_shared<Shader>(L"resources/shaders/vertex/VertexShader.cso", L"resources/shaders/pixel/gbuffer_write.cso");
		gBuffer_Shader->CreateLayout(Mesh::GetLayout(), Mesh::GetLayoutSize());
		Ref<Shader> deffered_Shader = std::make_shared<Shader>(L"resources/shaders/vertex/vertex_shader_no_transform.cso", L"resources/shaders/pixel/DefferedShading_ps.cso");
		deffered_Shader->CreateLayout(Mesh::GetLayout(), Mesh::GetLayoutSize());
		Ref<Shader> postprocess_Shader = std::make_shared<Shader>(L"resources/shaders/vertex/vertex_shader_no_transform.cso", L"resources/shaders/pixel/postprocess_ps.cso");
		postprocess_Shader->CreateLayout(Mesh::GetLayout(), Mesh::GetLayoutSize());
		shaderlib.Add("PBR-Forward-Shader", pbrShader);
		shaderlib.Add("PBR-Deffered-Shader", deffered_Shader);
		shaderlib.Add("GBuffer-Shader", gBuffer_Shader);
		shaderlib.Add("Shadow-Shader", shadowShader);
		shaderlib.Add("Post-Shader", postprocess_Shader);
		BYTE data[4] = { 255,255,255,255 };
		whiteTexture.CreateStack(1, 1, TextureFormat::RGBA8U,data);
		Shader::SetPSBuffer(LightCB, 0);
		Shader::SetVSBuffer(PassCB, 0);
		Shader::SetPSBuffer(MaterialCB, 1);
		auto Windata = GetWindowDataPtr();
		RendererBase::ChangeViewport(((WindowData*)(Windata))->width, ((WindowData*)(Windata))->height);
		RendererBase::SetCullMode(CullMode::Back);
		auto mainTarget = RendererBase::GetmainRenderTargetView();
		RendererBase::Getd3dDeviceContext()->OMSetRenderTargets(1, &mainTarget, RendererBase::GetDepthStencilView());
		delete ss;
		fbo.BindResource(8);
		ZeroMemory(&LightData, sizeof(LightData));
		lightIndexPtr = LightData.lights;

	}
	void Renderer::AddLight(const Light& light)
	{
		PT_PROFILE_FUNCTION();
		*lightIndexPtr = light;
		lightIndexPtr++;
		passConstants.numlights.x++;
	}
	void Renderer::BeginScene(PerspectiveCamera* cam) {
		PT_PROFILE_FUNCTION();
		viewproj = cam->GetViewProjectionMatrix();
		auto campos = cam->GetPosition();
		BrdfTex.BindResource(0, 1, 0);
		ifbo.BindResource(1);
		prefilter.BindResource(2);
		CameraData.viewProjection = viewproj;
		CameraData.viewPos = {campos.x,campos.y, campos.z, 1.f};
		LightCB.Update(&LightData, sizeof(LightData));
		UpdatePassConstants();
		
		whiteTexture.Bind(3);
		whiteTexture.Bind(4);
		whiteTexture.Bind(5);
	}
	void Renderer::BeginScene(RuntimeCamera* cam, const DirectX::XMMATRIX& transform) {
		PT_PROFILE_FUNCTION();
		DirectX::XMFLOAT4 campos;
		DirectX::XMStoreFloat4(&campos, transform.r[3]);
		DirectX::XMMATRIX view = DirectX::XMMatrixInverse(nullptr, transform);
		viewproj = DirectX::XMMatrixMultiplyTranspose(view, cam->GetProjection());
		ID3D11ShaderResourceView* pBrdfSRV = (ID3D11ShaderResourceView*)BrdfTex.GetSRV().ptr;
		RendererBase::Getd3dDeviceContext()->PSSetShaderResources(0, 1, &pBrdfSRV);
		ifbo.BindResource(1);
		prefilter.BindResource(2);
		CameraData.viewProjection = viewproj;
		CameraData.view = DirectX::XMMatrixTranspose(view);
		CameraData.viewPos = campos;
		LightCB.Update(&LightData, sizeof(LightData));
		UpdatePassConstants();
		whiteTexture.Bind(3);
		whiteTexture.Bind(4);
		whiteTexture.Bind(5);
	}
	void Renderer::BeginScene(EditorCamera& cam)
	{
		PT_PROFILE_FUNCTION();
		DirectX::XMFLOAT4 campos;
		DirectX::XMStoreFloat4(&campos, cam.GetPosition());
		viewproj = DirectX::XMMatrixTranspose(cam.GetViewProjection());
		ID3D11ShaderResourceView* pBrdfSRV = (ID3D11ShaderResourceView*)BrdfTex.GetSRV().ptr;
		RendererBase::Getd3dDeviceContext()->PSSetShaderResources(0, 1, &pBrdfSRV);
		ifbo.BindResource(1);
		prefilter.BindResource(2);
		CameraData.viewProjection = viewproj;
		CameraData.view = DirectX::XMMatrixTranspose(cam.GetViewMatrix());
		CameraData.viewPos = campos;
		LightCB.Update(&LightData, sizeof(LightData));
		UpdatePassConstants();
		whiteTexture.Bind(3);
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
		passConstants.numlights = {0,0,0,0};
	}
	void Renderer::Shutdown() {
		delete brdfSampler;
		delete shadowSampler;
		RendererBase::Shutdown();
	}
	void Renderer::Submit(Mesh* mesh, Shader* shader, Material* mat, int ID)
	{
		PT_PROFILE_FUNCTION();
		auto& VB = mesh->GetVertexBuffer(); 
		auto& IB = mesh->GetIndexBuffer();
		Buffer buffer = { &VB,&IB };
		MaterialStruct cb;
		auto diff = GetAssetManager()->GetTexture2DResource(mat->diffuseTex);
		auto rough = GetAssetManager()->GetTexture2DResource(mat->roughnessTex);
		auto metal = GetAssetManager()->GetTexture2DResource(mat->metallicTex);
		auto norm = GetAssetManager()->GetTexture2DResource(mat->normalTex);
		if (diff) { diff->Bind(3); }else { Renderer::whiteTexture.Bind(3); }
		if (rough) { rough->Bind(4); }else { Renderer::whiteTexture.Bind(4); }
		if (metal) { metal->Bind(5); }else { Renderer::whiteTexture.Bind(5); }
		if (norm) { norm->Bind(6); }else { Renderer::whiteTexture.Bind(6); }
		cb = { {mat->diffuseColor.x, mat->diffuseColor.y, mat->diffuseColor.z, 1.f},mat->metallic,mat->roughness,ID};
		MaterialCB.Update(&cb, sizeof(MaterialStruct));
		shader->Bind(ShaderType::Vertex);
		shader->Bind(ShaderType::Pixel);
		RendererBase::DrawIndexed(buffer);
	}
	void Renderer::UpdatePassConstants()
	{
		PT_PROFILE_FUNCTION();
		passConstants.EyePosW.x = CameraData.viewPos.x;
		passConstants.EyePosW.y = CameraData.viewPos.y; 
		passConstants.EyePosW.z = CameraData.viewPos.z;
		DirectX::XMStoreFloat4x4(&passConstants.View, CameraData.view);
		DirectX::XMStoreFloat4x4(&passConstants.ViewProj, CameraData.viewProjection);
		PassCB.Update(&passConstants, sizeof(PassConstants));
	}
}