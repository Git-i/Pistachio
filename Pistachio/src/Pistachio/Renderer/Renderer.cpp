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
Pistachio::RenderCubeMap Pistachio::Renderer::skybox = Pistachio::RenderCubeMap();
Pistachio::RenderCubeMap Pistachio::Renderer::irradianceSkybox = Pistachio::RenderCubeMap();
Pistachio::RenderCubeMap Pistachio::Renderer::prefilterSkybox = { Pistachio::RenderCubeMap() };
std::vector<Pistachio::RegularLight>   Pistachio::Renderer::RegularLightData = {};
std::vector<Pistachio::ShadowCastingLight>   Pistachio::Renderer::ShadowLightData = {};
std::vector<std::uint8_t> Pistachio::Renderer::LightSBCPU;
Pistachio::Renderer::CamerData Pistachio::Renderer::CameraData = {};
Pistachio::Texture2D Pistachio::Renderer::whiteTexture = Pistachio::Texture2D();
Pistachio::Material Pistachio::Renderer::DefaultMaterial = Pistachio::Material();
Pistachio::Material* Pistachio::Renderer::currentMat = nullptr;
Pistachio::Shader* Pistachio::Renderer::currentShader = nullptr;
Pistachio::SetInfo Pistachio::Renderer::eqShaderVS[6];
Pistachio::SetInfo Pistachio::Renderer::eqShaderPS;
Pistachio::SetInfo Pistachio::Renderer::irradianceShaderPS;
Pistachio::SetInfo Pistachio::Renderer::prefilterShaderVS[5];
Pistachio::DepthTexture Pistachio::Renderer::shadowMapAtlas;
static Pistachio::SamplerState* brdfSampler ;
static Pistachio::SamplerState* shadowSampler;
Pistachio::Shader* Pistachio::Renderer::eqShader;
Pistachio::Shader* Pistachio::Renderer::irradianceShader;
Pistachio::Shader* Pistachio::Renderer::brdfShader;
Pistachio::Shader* Pistachio::Renderer::prefilterShader;
Pistachio::SamplerHandle Pistachio::Renderer::defaultSampler;
RHI::Buffer*             Pistachio::Renderer::meshVertices; // all meshes in the scene?
RHI::Buffer*             Pistachio::Renderer::meshIndices;
uint32_t                 Pistachio::Renderer::vbFreeFastSpace;//free space for an immerdiate allocation
uint32_t                 Pistachio::Renderer::vbFreeSpace;   //total free space to consider reordering
uint32_t                 Pistachio::Renderer::vbCapacity;
Pistachio::FreeList      Pistachio::Renderer::vbFreeList;
uint32_t                 Pistachio::Renderer::ibFreeFastSpace;
uint32_t                 Pistachio::Renderer::ibFreeSpace;
uint32_t                 Pistachio::Renderer::ibCapacity;
uint32_t                 Pistachio::Renderer::cbCapacity;
uint32_t                 Pistachio::Renderer::cbFreeSpace;
uint32_t                 Pistachio::Renderer::cbFreeFastSpace;
Pistachio::FreeList      Pistachio::Renderer::cbFreeList;
Pistachio::FreeList      Pistachio::Renderer::ibFreeList;
Pistachio::FrameResource Pistachio::Renderer::resources[3];
std::vector<uint32_t>    Pistachio::Renderer::ibHandleOffsets;
std::vector<uint32_t>    Pistachio::Renderer::ibUnusedHandles;
std::vector<uint32_t>    Pistachio::Renderer::vbHandleOffsets;
std::vector<uint32_t>    Pistachio::Renderer::vbUnusedHandles;
std::vector<uint32_t>    Pistachio::Renderer::cbHandleOffsets;
std::vector<uint32_t>    Pistachio::Renderer::cbUnusedHandles; 

static uint32_t     numDirtyCBFrames;
 void (*Pistachio::Renderer::CBInvalidated)() = nullptr;
static const uint32_t VB_INITIAL_SIZE = 1024;
static const uint32_t IB_INITIAL_SIZE = 1024;
static const uint32_t INITIAL_NUM_LIGHTS = 20;
static const uint32_t INITIAL_NUM_OBJECTS = 20;

namespace Pistachio {
	void Renderer::Init(const char* skyboxFile)
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
		bufferDesc.usage = RHI::BufferUsage::IndexBuffer | RHI::BufferUsage::CopyDst | RHI::BufferUsage::CopySrc;
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
		cbFreeList = FreeList(RendererUtils::ConstantBufferElementSize(sizeof(TransformData)) * INITIAL_NUM_OBJECTS);

		cbCapacity = RendererUtils::ConstantBufferElementSize(sizeof(TransformData)) * INITIAL_NUM_OBJECTS;
		cbFreeFastSpace = cbCapacity;
		cbFreeSpace = cbCapacity;
		for (uint32_t i = 0; i < 3; i++)
		{
			PT_CORE_INFO("Initializing Frame Resources {0} of 3", i+1);
			PT_CORE_INFO("Creating Constant Buffer(s)");
			resources[i].transformBuffer.CreateStack(nullptr, cbCapacity);//better utilise the free 256 bytes
			resources[i].passCB.CreateStack(nullptr, RendererUtils::ConstantBufferElementSize(sizeof(PassConstants)));//we should still have about 50 free bytes
			PT_CORE_INFO("Creating Structured Buffer(s)");
			resources[i].LightSB.CreateStack(nullptr, sizeof(ShadowCastingLight) * INITIAL_NUM_LIGHTS);
			RendererBase::device->CreateDynamicDescriptor(
				RendererBase::heap, 
				&resources[i].transformBufferDesc, 
				RHI::DescriptorType::ConstantBufferDynamic, 
				RHI::ShaderStage::Vertex, 
				resources[i].transformBuffer.ID.Get(),
				0,
				256);
		}
		
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

		RHI::SamplerDesc sampler;
		sampler.AddressU = RHI::AddressMode::Clamp;
		sampler.AddressV = RHI::AddressMode::Clamp;
		sampler.AddressW = RHI::AddressMode::Clamp;
		sampler.anisotropyEnable = false;
		sampler.compareEnable = false;
		sampler.magFilter = RHI::Filter::Linear;
		sampler.minFilter = RHI::Filter::Linear;
		sampler.mipFilter = RHI::Filter::Linear;
		sampler.maxLOD = 0;
		sampler.minLOD = 0;
		sampler.mipLODBias = 0;

		defaultSampler = RendererBase::CreateSampler(&sampler);

		RHI::BlendMode blendMode{};
		blendMode.BlendAlphaToCoverage = false;
		blendMode.IndependentBlend = true;
		blendMode.blendDescs[0].blendEnable = false;
		RHI::DepthStencilMode dsMode{};
		dsMode.DepthEnable = false;
		dsMode.StencilEnable = false;
		RHI::RasterizerMode rsMode{};
		rsMode.cullMode = RHI::CullMode::None;
		rsMode.fillMode = RHI::FillMode::Solid;
		rsMode.topology = RHI::PrimitiveTopology::TriangleList;
		ShaderCreateDesc ShaderDesc{};
		ShaderDesc.VS = "resources/shaders/vertex/Compiled/equirectangular_to_cubemap_vs";
		ShaderDesc.PS = "resources/shaders/pixel/Compiled/equirectangular_to_cubemap_fs";
		ShaderDesc.BlendModes = &blendMode;
		ShaderDesc.DepthStencilModes = &dsMode;
		ShaderDesc.RasterizerModes = &rsMode;
		ShaderDesc.numBlendModes = 1;
		ShaderDesc.numDepthStencilModes = 1;
		ShaderDesc.numRasterizerModes = 1;
		ShaderDesc.InputDescription = Pistachio::Mesh::GetLayout();
		ShaderDesc.numInputs = Pistachio::Mesh::GetLayoutSize();
		ShaderDesc.NumRenderTargets = 1;
		ShaderDesc.RTVFormats[0] = RHI::Format::R16G16B16A16_FLOAT;

		eqShader = Shader::Create(&ShaderDesc);
		eqShader->GetPSShaderBinding(eqShaderPS, 1);
		for(uint32_t i = 0; i < 6; i++)
			eqShader->GetVSShaderBinding(eqShaderVS[i], 0);
		skybox.CreateStack(512, 512, 1, RHI::Format::R16G16B16A16_FLOAT);
		ShaderDesc.PS = "resources/shaders/pixel/Compiled/irradiance_fs";
		irradianceShader = Shader::Create(&ShaderDesc);
		irradianceShader->GetPSShaderBinding(irradianceShaderPS,1);
		irradianceSkybox.CreateStack(32, 32, 1, RHI::Format::R16G16B16A16_FLOAT);

		ShaderDesc.VS = "resources/shaders/vertex/Compiled/prefilter_vs";
		ShaderDesc.PS = "resources/shaders/pixel/Compiled/prefilter_fs";
		prefilterShader = Shader::Create(&ShaderDesc);
		for(uint32_t i = 0; i < 5; i++)
			prefilterShader->GetVSShaderBinding(prefilterShaderVS[i], 2);
		prefilterSkybox.CreateStack(128, 128, 5, RHI::Format::R16G16B16A16_FLOAT,RHI::TextureUsage::CopyDst);

		RHI::DepthStencilMode dsModeEnabledLEqual;
		dsModeEnabledLEqual.DepthEnable = true;
		dsModeEnabledLEqual.DepthFunc = RHI::ComparisonFunc::LessEqual;
		dsModeEnabledLEqual.DepthWriteMask = RHI::DepthWriteMask::All;
		dsModeEnabledLEqual.StencilEnable = false;

		//Fill the shader library
		ShaderDesc.VS = "resources/shaders/vertex/Compiled/VertexShader";
		ShaderDesc.PS = "resources/shaders/pixel/Compiled/gbuffer_write";
		ShaderDesc.DepthStencilModes = &dsModeEnabledLEqual;
		ShaderDesc.NumRenderTargets = 3;
		ShaderDesc.RTVFormats[0] = RHI::Format::R8G8B8A8_UNORM;
		ShaderDesc.RTVFormats[1] = ShaderDesc.RTVFormats[2] = RHI::Format::R16G16B16A16_FLOAT;
		ShaderDesc.DSVFormat = RHI::Format::D32_FLOAT;
		//blend mode doesnt change for gbuffer shader
		RHI::RootParameterDesc rpDesc[3];
		rpDesc[0].type = RHI::RootParameterType::DynamicDescriptor;
		rpDesc[0].dynamicDescriptor.setIndex = 1;
		rpDesc[0].dynamicDescriptor.stage = RHI::ShaderStage::Vertex;
		rpDesc[0].dynamicDescriptor.type = RHI::DescriptorType::ConstantBufferDynamic;
		RHI::DescriptorRange range;
		range.BaseShaderRegister = 0;
		range.numDescriptors = 1;
		range.stage = RHI::ShaderStage::Vertex;
		range.type = RHI::DescriptorType::ConstantBuffer;
		rpDesc[1].type = RHI::RootParameterType::DescriptorTable;
		rpDesc[2].type = RHI::RootParameterType::DescriptorTable;
		rpDesc[1].descriptorTable.setIndex = 0;
		rpDesc[1].descriptorTable.numDescriptorRanges = 1;
		rpDesc[1].descriptorTable.ranges = &range;
		RHI::DescriptorRange ranges[6];
		for (int i = 0; i < 6; i++) ranges[i].BaseShaderRegister = i;
		ranges[0].type = RHI::DescriptorType::SampledTexture;
		ranges[1].type = RHI::DescriptorType::SampledTexture;
		ranges[2].type = RHI::DescriptorType::SampledTexture;
		ranges[3].type = RHI::DescriptorType::SampledTexture;
		ranges[0].numDescriptors = 
		ranges[1].numDescriptors = 
		ranges[2].numDescriptors = 
		ranges[3].numDescriptors = 
		ranges[4].numDescriptors = 
		ranges[5].numDescriptors = 1;
		ranges[0].stage =
		ranges[1].stage =
		ranges[2].stage =
		ranges[3].stage =
		ranges[4].stage =
		ranges[5].stage = RHI::ShaderStage::Pixel;
		ranges[4].type = RHI::DescriptorType::Sampler;
		ranges[5].type = RHI::DescriptorType::ConstantBuffer;
		rpDesc[2].descriptorTable.numDescriptorRanges = 6;
		rpDesc[2].descriptorTable.ranges = ranges;
		rpDesc[2].descriptorTable.setIndex = 2;
		RHI::RootSignatureDesc rsDesc;
		rsDesc.numRootParameters = 3;
		rsDesc.rootParameters = rpDesc;
		RHI::DescriptorSetLayout* layouts[3]{};
		RHI::RootSignature* rs;
		RendererBase::device->CreateRootSignature(&rsDesc, &rs, layouts);
		shaderlib.Add("GBuffer-Shader", std::shared_ptr<Shader>(Shader::CreateWithRs(&ShaderDesc,rs,layouts,3)));
		rs->Release();
		ShaderDesc.VS = "resources/shaders/vertex/Compiled/VertexShader";
		ShaderDesc.PS = nullptr;
		ShaderDesc.NumRenderTargets = 0;
		shaderlib.Add("Shadow-Shader", std::shared_ptr<Shader>(Shader::Create(&ShaderDesc)));
		//for this we dont need depth testing
		ShaderDesc.DepthStencilModes = &dsMode;
		ShaderDesc.NumRenderTargets = 1;
		ShaderDesc.VS = "resources/shaders/vertex/Compiled/vertex_shader_no_transform";
		ShaderDesc.PS = "resources/shaders/pixel/Compiled/DefferedShading_fs";
		shaderlib.Add("PBR-Deferred-Shader", std::shared_ptr<Shader>(Shader::Create(&ShaderDesc)));
		BYTE data[4] = { 255,255,255,255 };
		whiteTexture.CreateStack(1, 1, RHI::Format::R8G8B8A8_UNORM,data);
		LightSB.Bind(7);
		//Shader::SetVSBuffer(PassCB, 0);
		//Shader::SetPSBuffer(MaterialCB, 1);
		auto Windata = GetWindowDataPtr();
		RendererBase::ChangeViewport(((WindowData*)(Windata))->width, ((WindowData*)(Windata))->height);
		RendererBase::SetCullMode(CullMode::Back);
		ChangeSkybox(skyboxFile);
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
		//the skybox texture
		RenderGraph skyboxRG(1);
		static Texture2D tex;
		tex.CreateStack(filename, RHI::Format::R32G32B32A32_FLOAT);

		static Mesh cube;
		static Mesh plane;
		cube.CreateStack("cube.obj");
		plane.CreateStack("plane.obj");


		//probably remove this
		struct CameraStruct {
			DirectX::XMMATRIX viewproj;
			DirectX::XMMATRIX transform;
			float roughness;
		} camerabufferData;
		static ConstantBuffer CameraCB;
		static ConstantBuffer PrefilterCB;
		PrefilterCB.CreateStack(nullptr, 256 * 5);
		CameraCB.CreateStack(nullptr, 256*6);//add padding
		
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
		for (uint32_t i = 0; i < 6; i++)
		{
			camerabufferData = { DirectX::XMMatrixMultiplyTranspose(captureViews[i], captureProjection), DirectX::XMMatrixIdentity(), 0 };
			CameraCB.Update(&camerabufferData, sizeof(camerabufferData), 256 * i);
			BufferBindingUpdateDesc bufferDesc;
			bufferDesc.buffer = CameraCB.GetID();
			bufferDesc.offset = 256 * i;
			bufferDesc.size = sizeof(Matrix4) * 2;
			bufferDesc.type = RHI::DescriptorType::ConstantBuffer;
			eqShaderVS[i].UpdateBufferBinding(&bufferDesc, 0);
		}
		for (uint32_t i = 0; i < 5; i++)
		{
			float roughness = (float)i / 4.f;
			PrefilterCB.Update(&roughness,sizeof(float),256*i);
			BufferBindingUpdateDesc bufferDesc;
			bufferDesc.buffer = PrefilterCB.GetID();
			bufferDesc.offset = 256 * i;
			bufferDesc.size = sizeof(float);
			bufferDesc.type = RHI::DescriptorType::ConstantBuffer;
			prefilterShaderVS[i].UpdateBufferBinding(&bufferDesc,0);
		}
		RendererBase::ChangeViewport(512, 512);
		//eqShader->Bind();
		eqShaderPS.UpdateTextureBinding(tex.GetView(), 0);
		eqShaderPS.UpdateSamplerBinding(defaultSampler, 1);
		irradianceShaderPS.UpdateTextureBinding(skybox.GetView(), 0);
		irradianceShaderPS.UpdateSamplerBinding(defaultSampler, 1);

		Pistachio::RGTexture* skyboxFaces[6];
		Pistachio::RGTexture* irradianceSkyboxFaces[6];
		for (int i = 0; i < 6; i++)
		{
			skyboxFaces[i] = skyboxRG.CreateTexture(&skybox, i);
			irradianceSkyboxFaces[i] = skyboxRG.CreateTexture(&irradianceSkybox, i);
			AttachmentInfo info;
			info.loadOp = RHI::LoadOp::Load;
			info.format = RHI::Format::R16G16B16A16_FLOAT;
			info.texture = skyboxFaces[i];
			auto& eqpass = skyboxRG.AddPass(RHI::PipelineStage::ALL_GRAPHICS_BIT, "Equirectangular to cubemap");
			eqpass.AddColorOutput(&info);
			eqpass.SetPassArea({0,0,512,512});
			eqpass.SetShader(eqShader);
			eqpass.pass_fn = [&,i](RHI::GraphicsCommandList* list)
				{
					RHI::Viewport vp;
					vp.height = vp.width = 512;
					vp.minDepth = 0;
					vp.maxDepth = 1;
					vp.x = vp.y = 0;
					list->SetViewports(1, &vp);
					RHI::Area2D rect = { 0,0,512,512 };
					list->SetScissorRects(1, &rect);
					list->BindVertexBuffers(0, 1, &meshVertices->ID);
					list->BindIndexBuffer(meshIndices, 0);
					eqShader->ApplyBinding(list, eqShaderVS[i]);
					eqShader->ApplyBinding(list, eqShaderPS);
					Submit(list, cube.GetVBHandle(), cube.GetIBHandle(), sizeof(Vertex));
					
				};
			
		}
		for (uint32_t i = 0; i < 6; i++)
		{
			auto& irradiancePass = skyboxRG.AddPass(RHI::PipelineStage::ALL_GRAPHICS_BIT, "Irradiance Filter");
			AttachmentInfo info;
			info.loadOp = RHI::LoadOp::Load;
			info.format = RHI::Format::R16G16B16A16_FLOAT;
			for (uint32_t j = 0; j < 6; j++)
			{
				info.texture = skyboxFaces[j];
				irradiancePass.AddColorInput(&info);//inject pass dependency
			}
			irradiancePass.SetPassArea({ 0,0,32,32 });
			irradiancePass.SetShader(irradianceShader);
			AttachmentInfo irrInfo;
			irrInfo.loadOp = RHI::LoadOp::Load;
			irrInfo.format = RHI::Format::R16G16B16A16_FLOAT;
			irrInfo.texture = irradianceSkyboxFaces[i];
			irradiancePass.AddColorOutput(&irrInfo);
			irradiancePass.pass_fn = [&, i](RHI::GraphicsCommandList* list)
				{
					RHI::Viewport vp;
					vp.height = vp.width = 32;
					vp.minDepth = 0;
					vp.maxDepth = 1;
					vp.x = vp.y = 0;
					list->SetViewports(1, &vp);
					RHI::Area2D rect = { 0,0,32,32 };
					list->SetScissorRects(1, &rect);
					list->BindVertexBuffers(0, 1, &meshVertices->ID);
					list->BindIndexBuffer(meshIndices, 0);
					irradianceShader->ApplyBinding(list, eqShaderVS[i]);
					irradianceShader->ApplyBinding(list, irradianceShaderPS);
					Submit(list, cube.GetVBHandle(), cube.GetIBHandle(), sizeof(Vertex));
				};
		}
		//todo
		static RenderCubeMap prefilterMipLevels[5];
		RGTexture* prefilterRGTextures[5*6];
		RGTexture* prefilterDstTextures[5 * 6];
		const uint32_t mipLevels = 5;
		uint32_t mipFaceIndex =0;
		for (uint32_t mip = 0; mip < mipLevels; mip++)
		{
			uint32_t mipWidth  = (int)(128 * std::pow(0.5, mip));
			uint32_t mipHeight = (int)(128 * std::pow(0.5, mip));
			//create the cubemap
			prefilterMipLevels[mip].CreateStack(mipWidth, mipHeight, 1, RHI::Format::R16G16B16A16_FLOAT,RHI::TextureUsage::CopySrc);
			//for each face
			for (uint32_t i = 0; i < 6; i++)
			{
				prefilterRGTextures [mipFaceIndex] = skyboxRG.CreateTexture(&prefilterMipLevels[mip], i);
				prefilterDstTextures[mipFaceIndex] = skyboxRG.CreateTexture((Texture*)&prefilterSkybox, mip, true, i);
				auto& prefilterPass = skyboxRG.AddPass(RHI::PipelineStage::ALL_GRAPHICS_BIT, "Render To Texture");
				for (uint32_t j = 0; j < 6; j++)
				{
					AttachmentInfo info{};
					info.format = RHI::Format::R16G16B16A16_FLOAT;
					info.texture = skyboxFaces[j];
					prefilterPass.AddColorInput(&info);
				}
				AttachmentInfo info{};
				info.format = RHI::Format::R16G16B16A16_FLOAT;
				info.texture = prefilterRGTextures[mipFaceIndex];
				prefilterPass.AddColorOutput(&info);
				prefilterPass.SetPassArea({ 0,0,mipWidth,mipHeight });
				prefilterPass.SetShader(prefilterShader);
				mipFaceIndex++;
				prefilterPass.pass_fn = [&, i, mip,mipWidth,mipHeight](RHI::GraphicsCommandList* list)
					{
						RHI::Viewport vp;
						vp.height = vp.width = mipWidth;
						vp.minDepth = 0;
						vp.maxDepth = 1;
						vp.x = vp.y = 0;
						list->SetViewports(1, &vp);
						RHI::Area2D rect = { 0,0,mipWidth,mipHeight };
						list->SetScissorRects(1, &rect);
						list->BindVertexBuffers(0, 1, &meshVertices->ID);
						list->BindIndexBuffer(meshIndices, 0);
						prefilterShader->ApplyBinding(list, eqShaderVS[i]);
						prefilterShader->ApplyBinding(list, prefilterShaderVS[mip]);
						prefilterShader->ApplyBinding(list, irradianceShaderPS);
						Submit(list, cube.GetVBHandle(), cube.GetIBHandle(), sizeof(Vertex));
					};
			}
		}
		//after rendering, we copy
		for (uint32_t mip = 0; mip < 5; mip++)
		{
			//we can copy the entire cupe map?
			auto& copyPass = skyboxRG.AddPass(RHI::PipelineStage::TRANSFER_BIT, "Prefilter Copy");
			for (uint32_t j = 0; j < 5 * 6; j++)
			{
				AttachmentInfo input;
				input.format = RHI::Format::R16G16B16A16_FLOAT;
				input.loadOp = RHI::LoadOp::Load;
				input.texture = prefilterRGTextures[j];
				input.usage = AttachmentUsage::Copy;
				copyPass.AddColorInput(&input);
				AttachmentInfo output;
				output.format = RHI::Format::R16G16B16A16_FLOAT;
				output.loadOp = RHI::LoadOp::Load;
				output.texture = prefilterDstTextures[j];
				output.usage = AttachmentUsage::Copy;
				copyPass.AddColorOutput(&output);
			}
			copyPass.pass_fn = [&,mip](RHI::GraphicsCommandList* list)
				{
					uint32_t mipSize = (int)(128 * std::pow(0.5, mip));
					RHI::SubResourceRange srcRange;
					srcRange.FirstArraySlice = 0;
					srcRange.imageAspect = RHI::Aspect::COLOR_BIT;
					srcRange.IndexOrFirstMipLevel = 0;
					srcRange.NumArraySlices = 6;
					srcRange.NumMipLevels = 1;
					RHI::SubResourceRange dstRange;
					dstRange.FirstArraySlice = 0;
					dstRange.imageAspect = RHI::Aspect::COLOR_BIT;
					dstRange.IndexOrFirstMipLevel = mip;
					dstRange.NumArraySlices = 6;
					dstRange.NumMipLevels = 1;
					list->CopyTextureRegion(srcRange, dstRange, { 0,0,0 }, { 0,0,0 }, { mipSize,mipSize,1 }, prefilterMipLevels[mip].m_ID.Get(), prefilterSkybox.m_ID.Get());
				};
		}
		////RendererBase::Getd3dDeviceContext()->GenerateMips((ID3D11ShaderResourceView*)fbo.GetID().ptr);

		//struct {
		//	DirectX::XMMATRIX viewproj;
		//	DirectX::XMMATRIX transform;
		//	float roughness;
		//}PrefilterShaderConstBuffer;
		////prefilterShader->Bind();
		//unsigned int maxMipLevels = 5;
		//RenderTexture texture = {};
		////prefilter.CreateStack(128, 128, 5);
		//RenderTextureDesc desc;
		//desc.miplevels = 1;
		//desc.Attachments = { TextureFormat::RGBA16F };
		//ConstantBuffer pfCB;
		//pfCB.Create(&PrefilterShaderConstBuffer, sizeof(PrefilterShaderConstBuffer));
		//for (unsigned int mip = 0; mip < maxMipLevels; ++mip)
		//{
		//	unsigned int mipWidth =  (int)(128 * std::pow(0.5, mip));
		//	unsigned int mipHeight = (int)(128 * std::pow(0.5, mip));
		//	for (int i = 0; i < 6; ++i)
		//	{
		//		desc.width = mipWidth;
		//		desc.height = mipHeight;
		//		//texture.CreateStack(desc);
		//	}
		//	// reisze framebuffer according to mip-level size.
		//	RendererBase::ChangeViewport(mipWidth, mipHeight);
		//	D3D11_BOX sourceRegion;
		//	for (unsigned int i = 0; i < 6; ++i)
		//	{

		//		PrefilterShaderConstBuffer.viewproj = DirectX::XMMatrixTranspose(captureViews[i] * captureProjection);
		//		PrefilterShaderConstBuffer.transform = DirectX::XMMatrixTranspose(DirectX::XMMatrixIdentity());
		//		PrefilterShaderConstBuffer.roughness = (float)mip / (float)(maxMipLevels - 1);
		//		pfCB.Update(&PrefilterShaderConstBuffer, sizeof(PrefilterShaderConstBuffer), 0);
		//		//prefilterShader.SetVSBuffer(pfCB, 0);
		//		//texture.Bind();
		//		//texture.Clear(clearcolor, 0);
		//		//fbo.BindResource(1);
		//		//RendererBase::DrawIndexed(buffer);

		//		sourceRegion.left = 0;
		//		sourceRegion.right = mipWidth;
		//		sourceRegion.top = 0;
		//		sourceRegion.bottom = mipHeight;
		//		sourceRegion.front = 0;
		//		sourceRegion.back = 1;
		//		//Texture2D lmao = texture.GetRenderTexture(0);
		//		//prefilter.GetResource().CopyIntoRegion(lmao, 0, 0, sourceRegion.left, sourceRegion.right, sourceRegion.top, sourceRegion.bottom, mip, i);
		//	}
		//}
		//Pistachio::RenderTextureDesc descBRDF;
		//descBRDF.Attachments = { TextureFormat::RGBA16F };
		//descBRDF.height = 512;
		//descBRDF.width = 512;
		////BrdfTex.CreateStack(descBRDF);
		////brdfShader->Bind();
		//Pistachio::RendererBase::SetCullMode(CullMode::Front);
		////BrdfTex.Bind();
		////RendererBase::DrawIndexed(planebuffer);
		skyboxRG.NewFrame();
		skyboxRG.Execute();
		skyboxRG.SubmitToQueue();
		RendererBase::fence_vals[RendererBase::currentFrameIndex] = ++RendererBase::currentFrameIndex;
		RendererBase::mainFence->Wait(RendererBase::currentFenceVal);
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
	const RendererCBHandle Renderer::AllocateConstantBuffer(uint32_t size)
	{
		auto [a,b] = AllocateBuffer(cbFreeList, cbFreeSpace, cbFreeFastSpace, cbCapacity, &Renderer::GrowConstantBuffer, &Renderer::DefragmentConstantBuffer, cbHandleOffsets, cbUnusedHandles, nullptr, RendererUtils::ConstantBufferElementSize(size), nullptr);
		return { a,b };
	}
	RenderCubeMap& Pistachio::Renderer::GetSkybox()
	{
		return skybox;
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
			if (initialData)
			{
				RendererBase::PushBufferUpdate(*buffer, capacity - fast_space, initialData, size);
				RendererBase::FlushStagingBuffer();
			}
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
				if (initialData)
				{
					RendererBase::PushBufferUpdate(*buffer, space, initialData, size);
					RendererBase::FlushStagingBuffer();
				}
				handle.handle = AssignHandle(offsetsVector, freeHandlesVector, space);
				handle.size = size;
				free_space -= size;
				return handle;
			}
			PT_CORE_WARN("Defragmenting Buffer");
			defrag_fn();
			if (initialData)
			{
				RendererBase::PushBufferUpdate(*buffer, capacity - fast_space, initialData, size);
				RendererBase::FlushStagingBuffer();
			}
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
		meshVertices->Release();
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
		meshIndices->Release();
		meshIndices = newIB;
		ibFreeSpace += minSize + GROW_FACTOR;
		ibFreeFastSpace += minSize + GROW_FACTOR;
		ibCapacity = new_size;
		ibFreeList.Grow(new_size);
	}
	void Pistachio::Renderer::GrowConstantBuffer(uint32_t minExtraSize)
	{
		const uint32_t GROW_FACTOR = 20; //probably use a better, more size dependent method to determing this
		uint32_t new_size = cbCapacity + minExtraSize + GROW_FACTOR;
		RHI::BufferDesc desc;
		desc.size = new_size;
		desc.usage = RHI::BufferUsage::ConstantBuffer;
		RendererBase::mainFence->Wait(RendererBase::currentFenceVal);
		for (uint32_t i = 0; i < 3; i++)
		{
			RHI::Buffer* newCB;
			RHI::AutomaticAllocationInfo allocInfo;
			allocInfo.access_mode = RHI::AutomaticAllocationCPUAccessMode::Sequential;
			RendererBase::device->CreateBuffer(&desc, &newCB, 0, 0, &allocInfo, 0, RHI::ResourceType::Automatic);
			void* writePtr;
			void* readPtr;
			newCB->Map(&writePtr);
			resources[i].transformBuffer.ID->Map(&readPtr);
			memcpy(writePtr, readPtr, cbCapacity);
			resources[i].transformBuffer.ID->UnMap();
			newCB->UnMap();
			resources[i].transformBuffer.ID = newCB;
		}
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
	RHI::Buffer* Renderer::GetVertexBuffer()
	{
		return meshVertices;
	}
	RHI::Buffer* Renderer::GetIndexBuffer()
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
	void Renderer::DefragmentConstantBuffer()
	{
		RendererBase::mainFence->Wait(RendererBase::currentFenceVal);
		auto block = cbFreeList.GetBlockPtr();
		uint32_t nextFreeOffset = 0;
		void *ptr1, *ptr2, *ptr3;
		resources[0].transformBuffer.ID->Map(&ptr1);
		resources[1].transformBuffer.ID->Map(&ptr2);
		resources[2].transformBuffer.ID->Map(&ptr3);
		while (block)
		{
			//if block has gap from last block
			if (block->offset > nextFreeOffset)
			{
				//we use memmove to handle overlap possibility
				memmove((uint8_t*)ptr1 + nextFreeOffset, (uint8_t*)ptr1 + block->offset, block->size);
				memmove((uint8_t*)ptr2 + nextFreeOffset, (uint8_t*)ptr2 + block->offset, block->size);
				memmove((uint8_t*)ptr3 + nextFreeOffset, (uint8_t*)ptr3 + block->offset, block->size);
				nextFreeOffset += block->size;
			}
			block = block->next;
		}
		resources[0].transformBuffer.ID->UnMap();
		resources[1].transformBuffer.ID->UnMap();
		resources[2].transformBuffer.ID->UnMap();
		if(CBInvalidated)
			CBInvalidated(); // the scene would bind a function here to update all descriptor sets
	}
	const uint32_t Pistachio::Renderer::GetIBOffset(const RendererIBHandle handle)
	{
		return ibHandleOffsets[handle.handle];
	}
	const uint32_t Pistachio::Renderer::GetVBOffset(const RendererVBHandle handle)
	{
		return vbHandleOffsets[handle.handle];
	}
	void Pistachio::Renderer::PartialCBUpdate(RendererCBHandle handle, void* data, uint32_t offset, uint32_t size)
	{
		PT_CORE_ASSERT(offset+size <= handle.size);
		resources[RendererBase::currentFrameIndex].transformBuffer.Update(data, size, cbHandleOffsets[handle.handle] + offset);
	}
	void Pistachio::Renderer::FullCBUpdate(RendererCBHandle handle, void* data)
	{
		resources[RendererBase::currentFrameIndex].transformBuffer.Update(data, handle.size, cbHandleOffsets[handle.handle]);
	}
	RHI::Buffer* Pistachio::Renderer::GetConstantBuffer()
	{
		return resources[RendererBase::currentFrameIndex].transformBuffer.ID.Get();
	}
	const RHI::DynamicDescriptor* Pistachio::Renderer::GetCBDesc()
	{
		return resources[RendererBase::currentFrameIndex].transformBufferDesc;
	}
	const uint32_t Pistachio::Renderer::GetCBOffset(const RendererCBHandle handle)
	{
		return cbHandleOffsets[handle.handle];
	}
	void Pistachio::Renderer::Submit(RHI::GraphicsCommandList* list,const RendererVBHandle vb, const RendererIBHandle ib, uint32_t vertexStride)
	{
		list->DrawIndexed(ib.size / sizeof(uint32_t),
			1,
			ibHandleOffsets[ib.handle] / sizeof(uint32_t),
			vbHandleOffsets[vb.handle] / vertexStride, 0);
	}
	const Texture2D& Pistachio::Renderer::GetWhiteTexture()
	{
		return whiteTexture;
	}
	SamplerHandle Pistachio::Renderer::GetDefaultSampler()
	{
		return defaultSampler;
	}
}