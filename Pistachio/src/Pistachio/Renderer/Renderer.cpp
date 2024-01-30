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

RHI::Buffer*            Pistachio::Renderer::meshVertices; // all meshes in the scene?
RHI::Buffer*            Pistachio::Renderer::meshIndices;
uint32_t                Pistachio::Renderer::vbFreeFastSpace;//free space for an immerdiate allocation
uint32_t                Pistachio::Renderer::vbFreeSpace;   //total free space to consider reordering
uint32_t                Pistachio::Renderer::vbCapacity;
Pistachio::FreeList     Pistachio::Renderer::vbFreeList;
uint32_t                Pistachio::Renderer::ibFreeFastSpace;
uint32_t                Pistachio::Renderer::ibFreeSpace;
uint32_t                Pistachio::Renderer::ibCapacity;
Pistachio::FreeList     Pistachio::Renderer::ibFreeList;

std::vector<uint32_t> Pistachio::Renderer::ibHandleOffsets;
std::vector<uint32_t> Pistachio::Renderer::ibUnusedHandles;
std::vector<uint32_t> Pistachio::Renderer::vbHandleOffsets;
std::vector<uint32_t> Pistachio::Renderer::vbUnusedHandles;
static const uint32_t VB_INITIAL_SIZE = 1024;
static const uint32_t IB_INITIAL_SIZE = 1024;

namespace Pistachio {
	void Renderer::CreateConstantBuffers()
	{
		PT_PROFILE_FUNCTION();
		for (uint32_t i = 0; i < 3; i++)
		{

		}
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
		//Create constant and structured buffers needed for each frame in flight
		PT_PROFILE_FUNCTION();
		PT_CORE_INFO("Initializing Renderer");

		//vertex buffer
		PT_CORE_INFO("Creating Vertex Buffer");
		RHI::BufferDesc bufferDesc;
		bufferDesc.size = VB_INITIAL_SIZE;
		bufferDesc.usage = RHI::BufferUsage::VertexBuffer | RHI::BufferUsage::CopyDst | RHI::BufferUsage::CopySrc;
		RHI::AutomaticAllocationInfo bufferAllocInfo;
		bufferAllocInfo.access_mode = RHI::AutomaticAllocationCPUAccessMode::None;
		RendererBase::device->CreateBuffer(&bufferDesc, &meshVertices, 0, 0, &bufferAllocInfo, 0, RHI::ResourceType::Automatic);
		//index buffer
		PT_CORE_INFO("Creating Index Buffer");
		bufferDesc.size = IB_INITIAL_SIZE;
		bufferDesc.usage = RHI::BufferUsage::VertexBuffer | RHI::BufferUsage::CopyDst | RHI::BufferUsage::CopySrc;
		RendererBase::device->CreateBuffer(&bufferDesc, &meshIndices, 0, 0, &bufferAllocInfo, 0, RHI::ResourceType::Automatic);
		//set constants for vb and ib
		vbCapacity = VB_INITIAL_SIZE;
		vbFreeFastSpace = VB_INITIAL_SIZE;
		vbFreeSpace = VB_INITIAL_SIZE;
		//---------------------------
		ibCapacity = IB_INITIAL_SIZE;
		ibFreeFastSpace = IB_INITIAL_SIZE;
		ibFreeSpace = IB_INITIAL_SIZE;
		//Create Free lists
		vbFreeList = FreeList(VB_INITIAL_SIZE);
		ibFreeList = FreeList(IB_INITIAL_SIZE);
		

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

		//auto& cubeVB = cube.GetVertexBuffer();
		//auto& cubeIB = cube.GetIndexBuffer();
		//auto& planeVB = plane.GetVertexBuffer();
		//auto& planeIB = plane.GetIndexBuffer();

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
		//auto& VB = mesh->GetVertexBuffer(); 
		//auto& IB = mesh->GetIndexBuffer();
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
	const RendererVBHandle Renderer::AllocateVertexBuffer(uint32_t size,const void* initialData)
	{
		return AllocateBuffer(vbFreeList, vbFreeSpace, vbFreeFastSpace, vbCapacity, &Renderer::GrowMeshVertexBuffer,&Renderer::DefragmentMeshVertexBuffer, vbHandleOffsets,vbUnusedHandles,&meshVertices, size, initialData);
	}
	const RendererIBHandle Renderer::AllocateIndexBuffer(uint32_t size, const void* initialData)
	{
		auto [a,b] = AllocateBuffer(ibFreeList, ibFreeSpace, ibFreeFastSpace, ibCapacity, &Renderer::GrowMeshIndexBuffer, &Renderer::DefragmentMeshIndexBuffer,ibHandleOffsets,ibUnusedHandles,&meshIndices, size, initialData);
		return { a,b };
	}
	inline RendererVBHandle Renderer::AllocateBuffer(
		FreeList& flist, uint32_t& free_space, 
		uint32_t& fast_space, 
		uint32_t& capacity,
		decltype(&Renderer::GrowMeshVertexBuffer) grow_fn,
		decltype(&Renderer::DefragmentMeshVertexBuffer) defrag_fn,
		std::vector<uint32_t>& offsetsVector,
		std::vector<uint32_t>& freeHandlesVector,
		RHI::Buffer** buffer, 
		uint32_t size,
		const void* initialData)
	{
		RendererVBHandle handle;
		//check if we have immediate space available
		if (size < fast_space)
		{
			//allocate to buffer end
			//fast space is always at the end
			PT_CORE_ASSERT(flist.Allocate(capacity - fast_space, size) == 0);
			if (initialData) RendererBase::PushBufferUpdate(*buffer, capacity - fast_space, initialData, size);
			RendererBase::FlushStagingBuffer();
			handle.handle = AssignHandle(offsetsVector,freeHandlesVector,capacity - fast_space);
			handle.size = size;
			fast_space -= size;
			free_space -= size;
			return handle;
		}
		//if not, check if we have space at all
		else if (size < free_space)
		{
			//if we have space, check the free list to see if space is continuos
			if (auto space = flist.Find(size); space != UINT32_MAX)
			{
				//allocate
				PT_CORE_ASSERT(flist.Allocate(space, size) == 0);
				if (initialData) RendererBase::PushBufferUpdate(*buffer, space, initialData, size);
				RendererBase::FlushStagingBuffer();
				handle.handle = AssignHandle(offsetsVector, freeHandlesVector, space);
				handle.size = size;
				free_space -= size;
				return handle;
			}
			PT_CORE_WARN("Defragmenting Buffer");
			defrag_fn();
			if (initialData) RendererBase::PushBufferUpdate(*buffer, capacity - fast_space, initialData, size);
			handle.handle = AssignHandle(offsetsVector,freeHandlesVector,capacity - fast_space);
			handle.size = size;
			fast_space -= size;
			free_space -= size;
			return handle;
			//allocate to buffer end
		}
		else
		{
			PT_CORE_WARN("Growing Buffer");
			grow_fn(size);
			//growth doesnt guarantee that the free space is "Fast" it just guarantees we'll have enough space for the op
			return AllocateBuffer(flist, free_space, fast_space, capacity,grow_fn,defrag_fn,offsetsVector,freeHandlesVector,buffer,size, initialData);
			//allocate to buffer end
		}
		return handle;
	}
	uint32_t Renderer::AssignHandle(std::vector<uint32_t>& offsetsVector, std::vector<uint32_t>& freeHandlesVector, std::uint32_t offset)
	{
		if (freeHandlesVector.empty())
		{
			offsetsVector.push_back(offset);
			return offsetsVector.size() - 1;
		}
		uint32_t handle = freeHandlesVector[freeHandlesVector.size()-1];
		offsetsVector[handle] = offset;
		freeHandlesVector.pop_back();
		return handle;
	}
	void Renderer::GrowMeshVertexBuffer(uint32_t minSize)
	{
		const uint32_t GROW_FACTOR = 20; //probably use a better, more size dependent method to determing this
		uint32_t new_size = vbCapacity + minSize + GROW_FACTOR;
		RHI::BufferDesc desc;
		desc.size = new_size;
		desc.usage = RHI::BufferUsage::VertexBuffer | RHI::BufferUsage::CopyDst | RHI::BufferUsage::CopySrc;
		RHI::Buffer* newVB;
		RHI::AutomaticAllocationInfo allocInfo;
		allocInfo.access_mode = RHI::AutomaticAllocationCPUAccessMode::None;
		RendererBase::device->CreateBuffer(&desc, &newVB, 0, 0, &allocInfo, 0, RHI::ResourceType::Automatic);
		//Queue it with staging stuff
		RendererBase::stagingCommandList->CopyBufferRegion(0, 0, vbCapacity, meshVertices, newVB);
		//wait until copy is finished?
		RendererBase::FlushStagingBuffer();
		//before destroying old buffer, wait for old frames to render
		RendererBase::mainFence->Wait(RendererBase::currentFenceVal);
		meshVertices->Destroy();
		meshVertices = newVB;
		vbFreeSpace += minSize + GROW_FACTOR;
		vbFreeFastSpace += minSize + GROW_FACTOR;
		vbCapacity = new_size;
		vbFreeList.Grow(new_size);
	}
	void Renderer::GrowMeshIndexBuffer(uint32_t minSize)
	{
		const uint32_t GROW_FACTOR = 20; //probably use a better, more size dependent method to determing this
		uint32_t new_size = ibCapacity + minSize + GROW_FACTOR;
		RHI::BufferDesc desc;
		desc.size = new_size;
		desc.usage = RHI::BufferUsage::IndexBuffer | RHI::BufferUsage::CopyDst | RHI::BufferUsage::CopySrc;
		RHI::Buffer* newIB;
		RHI::AutomaticAllocationInfo allocInfo;
		allocInfo.access_mode = RHI::AutomaticAllocationCPUAccessMode::None;
		RendererBase::device->CreateBuffer(&desc, &newIB, 0, 0, &allocInfo, 0, RHI::ResourceType::Automatic);
		//Queue it with staging stuff
		RendererBase::stagingCommandList->CopyBufferRegion(0, 0, ibCapacity, meshIndices, newIB);
		//wait until copy is finished?
		RendererBase::FlushStagingBuffer();
		//before destroying old buffer, wait for old frames to render
		RendererBase::mainFence->Wait(RendererBase::currentFenceVal);
		meshIndices->Destroy();
		meshIndices = newIB;
		ibFreeSpace += minSize + GROW_FACTOR;
		ibFreeFastSpace += minSize + GROW_FACTOR;
		ibCapacity = new_size;
		ibFreeList.Grow(new_size);
	}
	void Pistachio::Renderer::FreeVertexBuffer(const RendererVBHandle handle)
	{
		vbFreeSpace += handle.size;
		vbFreeList.DeAllocate(vbHandleOffsets[handle.handle], handle.size);
		vbUnusedHandles.push_back(handle.handle);
	}
	void Renderer::FreeIndexBuffer(const RendererIBHandle handle)
	{
		ibFreeSpace += handle.size;
		ibFreeList.DeAllocate(ibHandleOffsets[handle.handle], handle.size);
		ibUnusedHandles.push_back(handle.handle);
	}
	const RHI::Buffer* Renderer::GetVertexBuffer()
	{
		return meshVertices;
	}
	const RHI::Buffer* Renderer::GetIndexBuffer()
	{
		return meshIndices;
	}
	void Renderer::DefragmentMeshVertexBuffer()
	{
		auto block = vbFreeList.GetBlockPtr();
		uint32_t nextFreeOffset = 0;
		while (block)
		{
			if (block->offset > nextFreeOffset)
			{
				RendererBase::stagingCommandList->CopyBufferRegion(block->offset, nextFreeOffset, block->size, meshVertices, meshVertices);
				nextFreeOffset += block->size;
			}
			block = block->next;
		}
		vbFreeList.Reset();
		//fill up the beginning of the free list with the memory size we just copied
		vbFreeList.Allocate(0, nextFreeOffset);
		vbFreeFastSpace = vbFreeSpace;//after defrag all free space is fast space
		/*
		 *Is that a safe assumptions;
		 *we dont flush for every copy, because we asssume copies are done in order, so memory won't get overritten
		 */
		RendererBase::FlushStagingBuffer();
		
	}
	void Renderer::DefragmentMeshIndexBuffer()
	{
		auto block = ibFreeList.GetBlockPtr();
		uint32_t nextFreeOffset = 0;
		while (block)
		{
			if (block->offset > nextFreeOffset)
			{
				RendererBase::stagingCommandList->CopyBufferRegion(block->offset, nextFreeOffset, block->size, meshIndices, meshIndices);
				nextFreeOffset += block->size;
			}
			block = block->next;
		}
		ibFreeList.Reset();
		//fill up the beginning of the free list with the memory size we just copied
		ibFreeList.Allocate(0, nextFreeOffset);
		ibFreeFastSpace = vbFreeSpace;//after defrag all free space is fast space
		/*
		 *Is that a safe assumptions;
		 *we dont flush for every copy, because we asssume copies are done in order, so memory won't get overritten
		 */
		RendererBase::FlushStagingBuffer();

	}
	const uint32_t Pistachio::Renderer::GetIBOffset(const RendererIBHandle handle)
	{
		return ibHandleOffsets[handle.handle];
	}
	const uint32_t Pistachio::Renderer::GetVBOffset(const RendererVBHandle handle)
	{
		return vbHandleOffsets[handle.handle];
	}
}