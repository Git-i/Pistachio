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
Pistachio::StructuredBuffer Pistachio::Renderer::LightSB;
std::vector<Pistachio::ConstantBuffer> Pistachio::Renderer::TransformationBuffer;
Pistachio::PassConstants Pistachio::Renderer::passConstants;
Pistachio::ConstantBuffer Pistachio::Renderer::PassCB = {};
Pistachio::RenderCubeMap Pistachio::Renderer::fbo = Pistachio::RenderCubeMap();
Pistachio::RenderCubeMap Pistachio::Renderer::ifbo = Pistachio::RenderCubeMap();
Pistachio::RenderCubeMap Pistachio::Renderer::prefilter = { Pistachio::RenderCubeMap() };
std::vector<Pistachio::RegularLight>   Pistachio::Renderer::RegularLightData = {};
std::vector<Pistachio::ShadowCastingLight>   Pistachio::Renderer::ShadowLightData = {};
std::vector<std::uint8_t> Pistachio::Renderer::LightSBCPU;
Pistachio::Renderer::CamerData Pistachio::Renderer::CameraData = {};
Pistachio::Texture2D Pistachio::Renderer::whiteTexture = Pistachio::Texture2D();
Pistachio::Material Pistachio::Renderer::DefaultMaterial = Pistachio::Material();
Pistachio::Material* Pistachio::Renderer::currentMat = nullptr;
Pistachio::Shader* Pistachio::Renderer::currentShader = nullptr;
Pistachio::ShadowMap Pistachio::Renderer::shadowMapAtlas;
static Pistachio::SamplerState* brdfSampler ;
static Pistachio::SamplerState* shadowSampler;
Pistachio::Shader* Pistachio::Renderer::eqShader;
Pistachio::Shader* Pistachio::Renderer::irradianceShader;
Pistachio::Shader* Pistachio::Renderer::brdfShader;
Pistachio::Shader* Pistachio::Renderer::prefilterShader;

namespace Pistachio {
	void Renderer::CreateConstantBuffers()
	{
		PT_PROFILE_FUNCTION();
		MaterialStruct cb;
		MaterialCB.Create(nullptr, sizeof(MaterialStruct));
		LightSB.CreateStack(nullptr, sizeof(ShadowCastingLight) * 256,16); //todo : make light buffer dynamic
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
		std::cout << "renderer" << std::endl;
		CreateConstantBuffers();
		
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

		ShaderCreateDesc ShaderDesc{};
		ShaderDesc.VS = "resources/shaders/vertex/equirectangular_to_cubemap_vs";
		ShaderDesc.PS = "resources/shaders/pixel/equirectangular_to_cubemap_fs";
		//create the pbr skybox shaders
		// 
		//Ref<Shader> pbrShader = std::make_shared<Shader>(L"resources/shaders/vertex/VertexShader.cso", L"resources/shaders/pixel/PixelShader.cso");
		//pbrShader->CreateLayout(Mesh::GetLayout(), Mesh::GetLayoutSize());
		//Ref<Shader> shadowShader = std::make_shared<Shader>(L"resources/shaders/vertex/shadow_vs.cso", L"resources/shaders/pixel/Shadow_ps.cso", L"resources/shaders/geometry/shadow_gs.cso");
		//shadowShader->CreateLayout(Mesh::GetLayout(), Mesh::GetLayoutSize());
		//Ref<Shader> gBuffer_Shader = std::make_shared<Shader>(L"resources/shaders/vertex/VertexShader.cso", L"resources/shaders/pixel/gbuffer_write.cso");
		//gBuffer_Shader->CreateLayout(Mesh::GetLayout(), Mesh::GetLayoutSize());
		//Ref<Shader> gBuffer_Shader_shadowMap = std::make_shared<Shader>(L"resources/shaders/vertex/VertexShader.cso", L"resources/shaders/pixel/gbuffer_write_shadowMap.cso");
		//gBuffer_Shader_shadowMap->CreateLayout(Mesh::GetLayout(), Mesh::GetLayoutSize());
		//Ref<Shader> deffered_Shader = std::make_shared<Shader>(L"resources/shaders/vertex/vertex_shader_no_transform.cso", L"resources/shaders/pixel/DefferedShading_ps.cso");
		//deffered_Shader->CreateLayout(Mesh::GetLayout(), Mesh::GetLayoutSize());
		//Ref<Shader> postprocess_Shader = std::make_shared<Shader>(L"resources/shaders/vertex/vertex_shader_no_transform.cso", L"resources/shaders/pixel/postprocess_ps.cso");
		//postprocess_Shader->CreateLayout(Mesh::GetLayout(), Mesh::GetLayoutSize());
		//shaderlib.Add("PBR-Forward-Shader", pbrShader);
		//shaderlib.Add("PBR-Deffered-Shader", deffered_Shader);
		//shaderlib.Add("GBuffer-Shader", gBuffer_Shader);
		//shaderlib.Add("GBuffer-Shader-Shadow-Map", gBuffer_Shader_shadowMap);
		//shaderlib.Add("Shadow-Shader", shadowShader);
		//shaderlib.Add("Post-Shader", postprocess_Shader);
		BYTE data[4] = { 255,255,255,255 };
		whiteTexture.CreateStack(1, 1, RHI::Format::R8G8B8A8_UNORM,data);
		LightSB.Bind(7);
		//Shader::SetVSBuffer(PassCB, 0);
		//Shader::SetPSBuffer(MaterialCB, 1);
		auto Windata = GetWindowDataPtr();
		RendererBase::ChangeViewport(((WindowData*)(Windata))->width, ((WindowData*)(Windata))->height);
		RendererBase::SetCullMode(CullMode::Back);
		//auto mainTarget = RendererBase::GetmainRenderTargetView();
		//RendererBase::Getd3dDeviceContext()->OMSetRenderTargets(1, &mainTarget, RendererBase::GetDepthStencilView());
		delete ss;
		//fbo.BindResource(8);
		//shadowMapAtlas.Create(4096);
		//DefaultMaterial.Initialize();
		//DefaultMaterial.diffuseColor = Vector4(1);
		//DefaultMaterial.metallic = 0.f;
		//DefaultMaterial.roughness = 1.f;
		//DefaultMaterial.Update();

	}
	void Renderer::ChangeSkybox(const char* filename)
	{
		Texture2D tex;
		tex.CreateStack(filename, RHI::Format::R32G32B32A32_FLOAT);

		Mesh cube;
		Mesh plane;
		cube.CreateStack("cube.obj");
		plane.CreateStack("plane.obj");

		auto& cubeVB = cube.GetVertexBuffer();
		auto& cubeIB = cube.GetIndexBuffer();
		auto& planeVB = plane.GetVertexBuffer();
		auto& planeIB = plane.GetIndexBuffer();

		//probably remove this
		struct CameraStruct {
			DirectX::XMMATRIX viewproj;
			DirectX::XMMATRIX view;
			DirectX::XMFLOAT4 viewPos;
		} camerabufferData;
		ConstantBuffer CameraCB;
		CameraCB.Create(nullptr, sizeof(CameraStruct));
		
		//fbo.CreateStack(512, 512, 6);
		float clearcolor[] = { 0.7f, 0.7f, 0.7f, 1.0f };
		DirectX::XMMATRIX captureProjection = DirectX::XMMatrixPerspectiveFovLH(DirectX::XMConvertToRadians(90.0f), 1.0f, 0.1f, 10.0f);
		DirectX::XMMATRIX captureViews[] =
		{
			DirectX::XMMatrixLookAtLH(DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f), DirectX::XMVectorSet(1.0f,  0.0f,  0.0f, 1.0f), DirectX::XMVectorSet(0.0f, -1.0f,  0.0f, 1.0f)),
			DirectX::XMMatrixLookAtLH(DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f), DirectX::XMVectorSet(-1.0f,  0.0f,  0.0f, 1.0f), DirectX::XMVectorSet(0.0f, -1.0f,  0.0f, 1.0f)),
			DirectX::XMMatrixLookAtLH(DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f), DirectX::XMVectorSet(0.0f, -1.0f,  0.0f, 1.0f), DirectX::XMVectorSet(0.0f,  0.0f,  1.0f, 1.0f)),
			DirectX::XMMatrixLookAtLH(DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f), DirectX::XMVectorSet(0.0f,  1.0f,  0.0f, 1.0f), DirectX::XMVectorSet(0.0f,  0.0f, -1.0f, 1.0f)),
			DirectX::XMMatrixLookAtLH(DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f), DirectX::XMVectorSet(0.0f,  0.0f, -1.0f, 1.0f), DirectX::XMVectorSet(0.0f, -1.0f,  0.0f, 1.0f)),
			DirectX::XMMatrixLookAtLH(DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f), DirectX::XMVectorSet(0.0f,  0.0f,  1.0f, 1.0f), DirectX::XMVectorSet(0.0f, -1.0f,  0.0f, 1.0f))
		};
		RendererBase::ChangeViewport(512, 512);
		eqShader->Bind();
		tex.Bind(0);
		for (int i = 0; i < 6; i++) {
			//fbo.Bind(i);
			//fbo.Clear(clearcolor, i);


			DirectX::XMFLOAT4 campos = { 0.0, 0.0, 0.0, 1.0 };
			camerabufferData = { DirectX::XMMatrixMultiplyTranspose(captureViews[i], captureProjection), DirectX::XMMatrixIdentity(), campos };
			CameraCB.Update(&camerabufferData, sizeof(camerabufferData), 0);
			//eqShader.SetVSBuffer(CameraCB, 0);
			//RendererBase::DrawIndexed(buffer);
		}
		//RendererBase::Getd3dDeviceContext()->GenerateMips((ID3D11ShaderResourceView*)fbo.GetID().ptr);
		//ifbo.CreateStack(32, 32);
		RendererBase::ChangeViewport(32, 32);
		//irradianceShader.Bind();
		for (int i = 0; i < 6; i++)
		{

			//ifbo.Bind(i);
			//ifbo.Clear(clearcolor, i);


			DirectX::XMFLOAT4 campos = { 0.0, 0.0, 0.0, 1.0 };
			camerabufferData = { DirectX::XMMatrixMultiplyTranspose(captureViews[i], captureProjection), DirectX::XMMatrixIdentity(),campos };
			CameraCB.Update(&camerabufferData, sizeof(camerabufferData), 0);
			//irradianceShader.SetVSBuffer(CameraCB, 0);
			//fbo.BindResource(1);
			//RendererBase::DrawIndexed(buffer);
		}
		struct {
			DirectX::XMMATRIX viewproj;
			DirectX::XMMATRIX transform;
			float roughness;
		}PrefilterShaderConstBuffer;
		prefilterShader->Bind();
		unsigned int maxMipLevels = 5;
		RenderTexture texture = {};
		//prefilter.CreateStack(128, 128, 5);
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
				//texture.CreateStack(desc);
			}
			// reisze framebuffer according to mip-level size.
			RendererBase::ChangeViewport(mipWidth, mipHeight);
			D3D11_BOX sourceRegion;
			for (unsigned int i = 0; i < 6; ++i)
			{

				PrefilterShaderConstBuffer.viewproj = DirectX::XMMatrixTranspose(captureViews[i] * captureProjection);
				PrefilterShaderConstBuffer.transform = DirectX::XMMatrixTranspose(DirectX::XMMatrixIdentity());
				PrefilterShaderConstBuffer.roughness = (float)mip / (float)(maxMipLevels - 1);
				pfCB.Update(&PrefilterShaderConstBuffer, sizeof(PrefilterShaderConstBuffer), 0);
				//prefilterShader.SetVSBuffer(pfCB, 0);
				//texture.Bind();
				//texture.Clear(clearcolor, 0);
				//fbo.BindResource(1);
				//RendererBase::DrawIndexed(buffer);

				sourceRegion.left = 0;
				sourceRegion.right = mipWidth;
				sourceRegion.top = 0;
				sourceRegion.bottom = mipHeight;
				sourceRegion.front = 0;
				sourceRegion.back = 1;
				//Texture2D lmao = texture.GetRenderTexture(0);
				//prefilter.GetResource().CopyIntoRegion(lmao, 0, 0, sourceRegion.left, sourceRegion.right, sourceRegion.top, sourceRegion.bottom, mip, i);
			}
		}
		Pistachio::RenderTextureDesc descBRDF;
		descBRDF.Attachments = { TextureFormat::RGBA16F };
		descBRDF.height = 512;
		descBRDF.width = 512;
		//BrdfTex.CreateStack(descBRDF);
		brdfShader->Bind();
		Pistachio::RendererBase::SetCullMode(CullMode::Front);
		//BrdfTex.Bind();
		//RendererBase::DrawIndexed(planebuffer);
	}
	void Renderer::AddLight(const Light& light)
	{
		PT_PROFILE_FUNCTION();
		RegularLightData.push_back(light);
		passConstants.numRegularlights++;
	}
	void Renderer::AddShadowCastingLight(const ShadowCastingLight& light)
	{
		PT_PROFILE_FUNCTION();
		ShadowLightData.push_back(light);
		passConstants.numShadowlights++;

	}
	void Renderer::BeginScene(PerspectiveCamera* cam) {
		PT_PROFILE_FUNCTION();
		viewproj = cam->GetViewProjectionMatrix();
		auto campos = cam->GetPosition();
		//BrdfTex.BindResource(0, 1, 0);
		//ifbo.BindResource(1);
		//prefilter.BindResource(2);
		CameraData.viewProjection = viewproj;
		CameraData.viewPos = {campos.x,campos.y, campos.z, 1.f};
		uint32_t regularLightByteSize = sizeof(RegularLight) * RegularLightData.size(), shadowLightByteSize = sizeof(ShadowCastingLight) * ShadowLightData.size();
		LightSBCPU.resize(shadowLightByteSize + regularLightByteSize);
		if (regularLightByteSize)
			memcpy(&LightSBCPU[0], RegularLightData.data(), regularLightByteSize);
		if (shadowLightByteSize)
			memcpy(&LightSBCPU[regularLightByteSize], ShadowLightData.data(), shadowLightByteSize);
		if(LightSBCPU.size())
			LightSB.Update(LightSBCPU.data(), shadowLightByteSize + regularLightByteSize,0);
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
		//ID3D11ShaderResourceView* pBrdfSRV = (ID3D11ShaderResourceView*)BrdfTex.GetSRV().ptr;
		//RendererBase::Getd3dDeviceContext()->PSSetShaderResources(0, 1, &pBrdfSRV);
		//ifbo.BindResource(1);
		//prefilter.BindResource(2);
		CameraData.viewProjection = viewproj;
		CameraData.view = DirectX::XMMatrixTranspose(view);
		CameraData.viewPos = campos;
		uint32_t regularLightByteSize = sizeof(RegularLight) * RegularLightData.size(), shadowLightByteSize = sizeof(ShadowCastingLight) * ShadowLightData.size();
		LightSBCPU.resize(shadowLightByteSize + regularLightByteSize);
		if (regularLightByteSize)
			memcpy(&LightSBCPU[0], RegularLightData.data(), regularLightByteSize);
		if (shadowLightByteSize)
			memcpy(&LightSBCPU[regularLightByteSize], ShadowLightData.data(), shadowLightByteSize);
		if (LightSBCPU.size())
			LightSB.Update(LightSBCPU.data(), shadowLightByteSize + regularLightByteSize,0);
		UpdatePassConstants();
		whiteTexture.Bind(3);
		whiteTexture.Bind(4);
		whiteTexture.Bind(5);
		currentMat = nullptr;
	}
	void Renderer::BeginScene(EditorCamera& cam)
	{
		PT_PROFILE_FUNCTION();
		DirectX::XMFLOAT4 campos;
		DirectX::XMStoreFloat4(&campos, cam.GetPosition());
		viewproj = DirectX::XMMatrixTranspose(cam.GetViewProjection());
		//ID3D11ShaderResourceView* pBrdfSRV = (ID3D11ShaderResourceView*)BrdfTex.GetSRV().ptr;
		//RendererBase::Getd3dDeviceContext()->PSSetShaderResources(0, 1, &pBrdfSRV);
		CameraData.viewProjection = viewproj;
		CameraData.view = DirectX::XMMatrixTranspose(cam.GetViewMatrix());
		CameraData.viewPos = campos;
		uint32_t regularLightByteSize = sizeof(RegularLight) * RegularLightData.size(), shadowLightByteSize = sizeof(ShadowCastingLight) * ShadowLightData.size();
		LightSBCPU.resize(shadowLightByteSize + regularLightByteSize);
		if (regularLightByteSize)
			memcpy(&LightSBCPU[0], RegularLightData.data(), regularLightByteSize);
		if(shadowLightByteSize)
			memcpy(&LightSBCPU[regularLightByteSize], ShadowLightData.data(), shadowLightByteSize);
		if (LightSBCPU.size())
			LightSB.Update(LightSBCPU.data(), shadowLightByteSize + regularLightByteSize,0);
		UpdatePassConstants();
		//ifbo.BindResource(1);
		//prefilter.BindResource(2);
		whiteTexture.Bind(3);
		whiteTexture.Bind(4);
		whiteTexture.Bind(5);
		LightSB.Bind(7);
		//fbo.BindResource(8);
		currentMat = nullptr;
		currentShader = nullptr;
	}
	void Renderer::EndScene()
	{
		PT_PROFILE_FUNCTION()
		RegularLightData.clear();
		ShadowLightData.clear();
		LightSBCPU.clear();
		passConstants.numRegularlights = 0;
		passConstants.numShadowlights = 0;
		auto data = ((WindowData*)GetWindowDataPtr());
		RendererBase::EndFrame();
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
		//Buffer buffer = { &VB,&IB };
		if ((mat == currentMat));
		else
		{
			mat->Bind();
			currentMat = mat;
		}
		if (shader == currentShader);
		else
		{
			//shader->Bind(ShaderType::Vertex);
			//shader->Bind(ShaderType::Pixel);
			currentShader = shader;
		}
		//RendererBase::DrawIndexed(buffer);
	}
	void Renderer::UpdatePassConstants()
	{
		PT_PROFILE_FUNCTION();
		passConstants.EyePosW.x = CameraData.viewPos.x;
		passConstants.EyePosW.y = CameraData.viewPos.y; 
		passConstants.EyePosW.z = CameraData.viewPos.z;
		DirectX::XMStoreFloat4x4(&passConstants.View, CameraData.view);
		DirectX::XMStoreFloat4x4(&passConstants.ViewProj, CameraData.viewProjection);
		PassCB.Update(&passConstants, sizeof(PassConstants),0);
	}
}