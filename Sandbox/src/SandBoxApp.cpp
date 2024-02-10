#define PISTACHIO_RENDER_API_DX11
#include "Pistachio.h"
#include "Pistachio/Core/Error.h"

#include "Pistachio/Core/EntryPoint.h"
RHI::Viewport vp;
struct
{
	Pistachio::Matrix4 view;
	Pistachio::Matrix4 invView;
	Pistachio::Matrix4 proj;
	Pistachio::Matrix4 invProj;
	Pistachio::Matrix4 viewProjection;
	Pistachio::Matrix4 transform = Pistachio::Matrix4(1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1);
	Pistachio::Vector4 viewPos =   Pistachio::Vector4(0, 10, -20, 1);
	Pistachio::Vector4 albedo =    Pistachio::Vector4(0.4, 0, 3.5, 1);
	float metallic = 0.f;
	float roughness = 1.f;
	float ao = 1.f;
}CBData;
decltype(CBData) CBData2;
class ExampleLayer : public Pistachio::Layer
{
public:
	ExampleLayer(const char* name) :
		Layer(name), cam(45.f, 16.f / 9.f, 0.1, 100.f),
		mesh("sphere.obj"),
		cube("cube.obj"),
		sphere(new Pistachio::Model("circle.obj")),
		graph(1)
	{
	}
	void OnUpdate(float delta) override
	{
		PT_PROFILE_FUNCTION();
		this->delta = delta;
		frame++;
		tex->InvalidateRTVHandle();
		tex->SetResource(Pistachio::RendererBase::GetBackBufferTexture(Pistachio::RendererBase::GetCurrentRTVIndex()));
		graph.NewFrame();
		graph.Execute();
		graph.SubmitToQueue();
		
	}
	void OnAttach() override
	{
		vp.x = 0;  vp.y = 0;
		vp.width =  1280;
		vp.height = 0720;
		vp.minDepth = 0.f;
		vp.maxDepth = 1.f;
		RHI::DepthStencilMode dsMode[2]{};
		dsMode[0].DepthEnable = true;
		dsMode[0].StencilEnable = false;
		dsMode[0].DepthFunc = RHI::ComparisonFunc::LessEqual;
		dsMode[0].DepthWriteMask = RHI::DepthWriteMask::All;
		dsMode[0].BackFace.DepthfailOp = RHI::StencilOp::DecrSat;

		dsMode[1] = dsMode[0];
		dsMode[1].DepthEnable = false;
		RHI::BlendMode blendMode{};
		blendMode.IndependentBlend = true;
		blendMode.blendDescs[0].blendEnable = false;
		RHI::RasterizerMode rsMode{};
		rsMode.fillMode = RHI::FillMode::Solid;
		rsMode.AntialiasedLineEnable = false;
		rsMode.conservativeRaster = false;
		rsMode.cullMode = RHI::CullMode::None;
		rsMode.depthBias = 0;
		rsMode.multisampleEnable = false;
		rsMode.topology = RHI::PrimitiveTopology::TriangleList;
		using namespace Pistachio;
		BufferLayout layout[3] = { 
			{"SEM0", BufferLayoutFormat::FLOAT3, 0 },
			{"SEM1", BufferLayoutFormat::FLOAT3, 3*4 },
			{"SEM2", BufferLayoutFormat::FLOAT2, 6*4 }
		};
		ShaderCreateDesc desc{};
		desc.VS = "C:/Dev/Repos/Pistachio/Sandbox/Shaders/Compiled/background_vs";
		desc.PS = "C:/Dev/Repos/Pistachio/Sandbox/Shaders/Compiled/background_fs";
		desc.numDepthStencilModes = 2;
		desc.DepthStencilModes = dsMode;
		desc.DSVFormat = RHI::Format::D32_FLOAT;
		desc.numBlendModes = 1;
		desc.RasterizerModes = &rsMode;
		desc.numRasterizerModes = 1;
		desc.numBlendModes = 1;
		desc.RTVFormats[0] = RHI::Format::B8G8R8A8_UNORM;
		desc.NumRenderTargets = 1;
		desc.numInputs = 3;
		desc.InputDescription = layout;
		desc.BlendModes = &blendMode;
		
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

		SamplerHandle samplerHandle = RendererBase::CreateSampler(&sampler);


		RHI::RootParameterDesc rpDesc[2];
		rpDesc[0].type = RHI::RootParameterType::DynamicDescriptor;
		rpDesc[0].dynamicDescriptor.stage = RHI::ShaderStage::Vertex;
		rpDesc[0].dynamicDescriptor.type = RHI::DescriptorType::ConstantBufferDynamic;
		rpDesc[0].dynamicDescriptor.setIndex = 0;
		RHI::DescriptorRange range[2];
		range[0].BaseShaderRegister = 0;
		range[0].numDescriptors = 1;
		range[0].stage = RHI::ShaderStage::Pixel;
		range[0].type = RHI::DescriptorType::SampledTexture;
		range[1].BaseShaderRegister = 1;
		range[1].numDescriptors = 1;
		range[1].stage = RHI::ShaderStage::Pixel;
		range[1].type = RHI::DescriptorType::Sampler;
		rpDesc[1].type = RHI::RootParameterType::DescriptorTable;
		rpDesc[1].descriptorTable.numDescriptorRanges = 2;
		rpDesc[1].descriptorTable.setIndex = 1;
		rpDesc[1].descriptorTable.ranges = range;

		RHI::DescriptorSetLayout* setlayout[2]{};

		RHI::RootSignatureDesc rsDesc;
		rsDesc.numRootParameters = 2;
		rsDesc.rootParameters = rpDesc;
		RHI::RootSignature* rs;
		RendererBase::Getd3dDevice()->CreateRootSignature(&rsDesc, &rs, setlayout);
		shad = Shader::CreateWithRs(&desc, rs, setlayout,2);
		rs->Release();
		RHI::DepthStencilMode currMode{};
		//dsMode->DepthFunc = RHI::ComparisonFunc::Always;
		shad->SetDepthStencilMode(dsMode, ShaderModeSetFlags::AutomaticallyCreate);
		
		CBData.viewProjection = cam.GetViewProjection().Transpose();
		cBuf1 = Renderer::AllocateConstantBuffer(sizeof(CBData));
		cBuf2 = Renderer::AllocateConstantBuffer(sizeof(CBData));
		rtex = RenderCubeMap::Create(512, 512, 1, RHI::Format::B8G8R8A8_UNORM);
		shad->GetPSShaderBinding(info, 1);
		info.UpdateTextureBinding(rtex->GetView(), 0);
		info.UpdateSamplerBinding(samplerHandle, 1);
		RendererBase::FlushStagingBuffer();
		auto& pass = graph.AddPass(RHI::PipelineStage::ALL_GRAPHICS_BIT, "Render Into CubeMap");
		tex = graph.CreateTexture(RendererBase::GetBackBufferTexture(RendererBase::GetCurrentRTVIndex()));
		auto dtex = graph.CreateTexture(RendererBase::GetDefaultDepthTexture());
		Pistachio::AttachmentInfo attachInfo;
		attachInfo.texture = tex;
		attachInfo.format = RHI::Format::B8G8R8A8_UNORM;
		pass.AddColorOutput(&attachInfo);
		pass.SetPassArea({ 0,0,1280,720 });
		attachInfo.texture = dtex;
		attachInfo.format = RHI::Format::D32_FLOAT;
		pass.SetDepthStencilOutput(&attachInfo);
		pass.pass_fn = [this](RHI::GraphicsCommandList* list) 
			{
				RHI::Area2D area;
				area.offset = { 0,0 };
				area.size = { 1280,720 };
				cam.OnUpdate(delta);
				auto mat = cam.GetViewProjection();
				mat = mat.Transpose();
				CBData.viewProjection = mat;
				CBData2 = CBData;
				CBData2.albedo = Pistachio::Vector4(1, 0, 0, 1);
				static float x = sinf((float)frame / 50.f) * 3;
				float old_x = x;
				x = sinf((float)frame / 50.f) * 3;
				float up = -1;
				if (old_x - x < 0) up = 1;
				CBData2.transform = Pistachio::Matrix4::CreateTranslation(x, sqrtf(9 - x * x) * up, 0).Transpose();
				CBData.transform = Pistachio::Matrix4::CreateTranslation(0, x, 0).Transpose();
				Pistachio::Renderer::FullCBUpdate(cBuf1, &CBData);
				Pistachio::Renderer::FullCBUpdate(cBuf2, &CBData2);
				list->SetViewports(1, &vp);
				list->SetScissorRects(1, &area);
				shad->Bind(list);
				list->BindVertexBuffers(0, 1, &Pistachio::Renderer::GetVertexBuffer()->ID);
				list->BindIndexBuffer(Pistachio::Renderer::GetIndexBuffer(), 0);
				list->BindDynamicDescriptor(shad->GetRootSignature(), Pistachio::Renderer::GetCBDesc(), 0, Pistachio::Renderer::GetCBOffset(cBuf1));
				shad->ApplyBinding(list, info);
				Pistachio::Renderer::Submit(list,mesh.meshes[0].GetVBHandle(), mesh.meshes[0].GetIBHandle(), sizeof(Pistachio::Vertex));
				list->BindDynamicDescriptor(shad->GetRootSignature(), Pistachio::Renderer::GetCBDesc(), 0, Pistachio::Renderer::GetCBOffset(cBuf2));
				if (frame < 3000 || frame > 5000)
				{
					Pistachio::Renderer::Submit(list, sphere->meshes[0].GetVBHandle(), sphere->meshes[0].GetIBHandle(), sizeof(Pistachio::Vertex));
				}
				else
				{
					if (frame == 3002)
						delete sphere;
					if (frame == 3004)
						sphere = new Pistachio::Model("circle.obj");
					Pistachio::Renderer::Submit(list, cube.meshes[0].GetVBHandle(), cube.meshes[0].GetIBHandle(), sizeof(Pistachio::Vertex));
				}

			};
	}
	void OnImGuiRender()
	{
		

	}
	void OnEvent(Pistachio::Event& event) override
	{
		Pistachio::EventDispatcher dispatcher(event);
		cam.OnEvent(event);
	}
	~ExampleLayer() {
	}
private:
	float delta = 0;
	int frame = 0;
	Pistachio::RenderCubeMap* rtex;
	Pistachio::RGTexture* tex;
	Pistachio::RenderGraph graph;
	Pistachio::RendererCBHandle cBuf1;
	Pistachio::RendererCBHandle cBuf2;
	Pistachio::Model mesh;
	Pistachio::Model* sphere;
	Pistachio::Model cube;
	Pistachio::Shader* envshader;
	Pistachio::Texture2D metalt;
	Pistachio::Texture2D albedot;
	Pistachio::EditorCamera cam;
	Pistachio::SetInfo info;
	Pistachio::Shader* shad;
};
class Sandbox : public Pistachio::Application
{
public:
	Sandbox() : Application("SandBoc") { PushLayer(new ExampleLayer("lol")); GetWindow().SetVsync(0); }
	~Sandbox() { }
private:
};

Pistachio::Application* Pistachio::CreateApplication()
{
	return new Sandbox;
}
