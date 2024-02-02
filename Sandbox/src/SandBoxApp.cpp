#define PISTACHIO_RENDER_API_DX11
#include "Pistachio.h"
#include "Pistachio/Core/Error.h"

#include "Pistachio/Core/EntryPoint.h"
RHI::Viewport vp;
struct
{
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
		Layer(name), cam(45.f, 16.f/9.f, 0.1, 100.f),
		mesh("sphere.obj"), 
		cube("cube.obj"), 
		sphere(new Pistachio::Model("circle.obj")),
		ocam(-10.f, 10.f, 10.f, -10.f, 1.6)
	{
	}
	void OnUpdate(float delta) override
	{
		PT_PROFILE_FUNCTION();
		static int frame = 0;
		frame++;
		RHI::Area2D area;
		area.offset = { 0,0 };
		area.size = {Pistachio::Application::Get().GetWindow().GetWidth(),Pistachio::Application::Get().GetWindow().GetHeight() };
		RHI::RenderingAttachmentDesc rdesc;
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
		CBData2.transform = Pistachio::Matrix4::CreateTranslation(x, sqrtf(9-x*x)*up, 0).Transpose();
		CBData.transform = Pistachio::Matrix4::CreateTranslation(0, x, 0).Transpose();
		Pistachio::Renderer::FullCBUpdate(cBuf1, &CBData);
		Pistachio::Renderer::FullCBUpdate(cBuf2, &CBData2);
		rdesc.clearColor = { 0.34,0.34,0.34,0.34 };
		rdesc.ImageView.val = Pistachio::RendererBase::GetRTVDescriptorHeap()->GetCpuHandle().val + (Pistachio::RendererBase::GetCurrentRTVIndex() * Pistachio::RendererBase::Getd3dDevice()->GetDescriptorHeapIncrementSize(RHI::DescriptorType::RTV));
		rdesc.loadOp = RHI::LoadOp::Clear;
		rdesc.storeOp = RHI::StoreOp::Store;
		RHI::RenderingAttachmentDesc depthDesc;
		depthDesc.clearColor = { 1,1,1,1 };
		depthDesc.ImageView = Pistachio::RendererBase::GetDSVDescriptorHeap()->GetCpuHandle();
		depthDesc.loadOp = RHI::LoadOp::Clear;
		depthDesc.storeOp = RHI::StoreOp::DontCare;
		RHI::RenderingBeginDesc desc;
		desc.numColorAttachments = 1;
		desc.pDepthStencilAttachment = &depthDesc;
		desc.renderingArea = area;
		desc.pColorAttachments = &rdesc;
		Pistachio::RendererBase::GetMainCommandList()->BeginRendering(&desc);
		Pistachio::RendererBase::GetMainCommandList()->SetViewports(1, &vp);
		Pistachio::RendererBase::GetMainCommandList()->SetScissorRects(1, &area);
		
		shad->Bind();
		Pistachio::RendererBase::GetMainCommandList()->BindVertexBuffers(0, 1, &Pistachio::Renderer::GetVertexBuffer()->ID);
		Pistachio::RendererBase::GetMainCommandList()->BindIndexBuffer(Pistachio::Renderer::GetIndexBuffer(), 0);
		Pistachio::RendererBase::GetMainCommandList()->BindDynamicDescriptor(shad->GetRootSignature(), Pistachio::Renderer::GetCBDesc(), 0, Pistachio::Renderer::GetCBOffset(cBuf1));
		Pistachio::RendererBase::GetMainCommandList()->BindDescriptorSet(shad->GetRootSignature(), set, 1);
		Pistachio::Renderer::Submit(mesh.meshes[0].GetVBHandle(), mesh.meshes[0].GetIBHandle(), sizeof(Pistachio::Vertex));
		Pistachio::RendererBase::GetMainCommandList()->BindDynamicDescriptor(shad->GetRootSignature(), Pistachio::Renderer::GetCBDesc(), 0, Pistachio::Renderer::GetCBOffset(cBuf2));
		if (frame < 3000 || frame > 5000)
		{
			Pistachio::Renderer::Submit(sphere->meshes[0].GetVBHandle(), sphere->meshes[0].GetIBHandle(), sizeof(Pistachio::Vertex));
		}
		else
		{
			if (frame == 3002)
				delete sphere;
			if (frame == 3004)
				sphere = new Pistachio::Model("circle.obj");
			Pistachio::Renderer::Submit(cube.meshes[0].GetVBHandle(), cube.meshes[0].GetIBHandle(), sizeof(Pistachio::Vertex));
		}
		

		Pistachio::RendererBase::GetMainCommandList()->EndRendering();
		
	}
	void OnAttach() override
	{
		vp.x = 0;  vp.y = 0;
		vp.width = 1280;
		vp.height = 720;
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
		desc.VS = "C:/Dev/Repos/Pistachio/Sandbox/Shaders/PBR_no_reflect_vs";
		desc.PS = "C:/Dev/Repos/Pistachio/Sandbox/Shaders/PBR_no_reflect_fs";
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
		

		RHI::RootParameterDesc rpDesc[2];
		rpDesc[0].type = RHI::RootParameterType::DynamicDescriptor;
		rpDesc[0].dynamicDescriptor.stage = RHI::ShaderStage::Vertex;
		rpDesc[0].dynamicDescriptor.type = RHI::DescriptorType::ConstantBufferDynamic;
		rpDesc[0].dynamicDescriptor.setIndex = 0;
		RHI::DescriptorRange range;
		range.BaseShaderRegister = 0;
		range.numDescriptors = 1;
		range.stage = RHI::ShaderStage::Pixel;
		range.type = RHI::DescriptorType::SampledTexture;
		rpDesc[1].type = RHI::RootParameterType::DescriptorTable;
		rpDesc[1].descriptorTable.numDescriptorRanges = 1;
		rpDesc[1].descriptorTable.setIndex = 1;
		rpDesc[1].descriptorTable.ranges = &range;

		RHI::DescriptorSetLayout* setlayout[2];

		RHI::RootSignatureDesc rsDesc;
		rsDesc.numRootParameters = 2;
		rsDesc.rootParameters = rpDesc;
		RHI::RootSignature* rs;
		RendererBase::Getd3dDevice()->CreateRootSignature(&rsDesc, &rs, setlayout);
		shad = Shader::CreateWithRs(&desc, rs);
		rs->Release();
		RHI::DepthStencilMode currMode{};
		
		//dsMode->DepthFunc = RHI::ComparisonFunc::Always;
		shad->SetDepthStencilMode(dsMode, ShaderModeSetFlags::AutomaticallyCreate);
		currMode.DepthEnable = true;
		currMode.DepthFunc = RHI::ComparisonFunc::LessEqual;
		currMode.DepthWriteMask = RHI::DepthWriteMask::All;
		currMode.StencilEnable = false;
		shad->SetDepthStencilMode(&currMode, ShaderModeSetFlags::AutomaticallyCreate);
		
		
		CBData.viewProjection = cam.GetViewProjection().Transpose();
		cBuf1 = Renderer::AllocateConstantBuffer(sizeof(CBData));
		cBuf2 = Renderer::AllocateConstantBuffer(sizeof(CBData));
		
		RendererBase::Getd3dDevice()->CreateDescriptorSets(RendererBase::GetMainDescriptorHeap(), 1, setlayout[1], &set);
		RHI::DescriptorTextureInfo tinfo;
		tinfo.dimension = RHI::ShaderResourceViewDimension::Texture2D;
		tinfo.texture = Renderer::GetWhiteTexture().GetView();
		RHI::DescriptorSetUpdateDesc updateDesc;
		updateDesc.arrayIndex = 0;
		updateDesc.binding = 0;
		updateDesc.bufferInfos = 0;
		updateDesc.numDescriptors = 1;
		updateDesc.type = RHI::DescriptorType::SampledTexture;
		updateDesc.textureInfos = &tinfo;
		RendererBase::Getd3dDevice()->UpdateDescriptorSets(1, &updateDesc, set);
		RendererBase::FlushStagingBuffer();
		int a = 9;
	}
	void OnImGuiRender()
	{
		

	}
	void OnEvent(Pistachio::Event& event) override
	{
		Pistachio::EventDispatcher dispatcher(event);
		cam.OnEvent(event);
		dispatcher.Dispatch<Pistachio::MouseScrolledEvent>(BIND_EVENT_FN(ExampleLayer::OnScroll));
	}
	bool OnScroll(Pistachio::MouseScrolledEvent& e)
	{
		//auto a = cam->GetPosition();
		//DirectX::XMFLOAT3 dir;
		//DirectX::XMStoreFloat3(&dir,DirectX::XMVectorSubtract(DirectX::XMVectorZero(), DirectX::XMLoadFloat3(&a)));
		//DirectX::XMStoreFloat3(&dir,DirectX::XMVector3Normalize(DirectX::XMLoadFloat3(&dir)));
		//DirectX::XMStoreFloat3(&dir, DirectX::XMVectorScale(DirectX::XMLoadFloat3(&dir), e.GetYOffset() / 100));
		//DirectX::XMStoreFloat3(&a, DirectX::XMVectorAdd(DirectX::XMLoadFloat3(&a), DirectX::XMLoadFloat3(&dir)));
		//if(!DirectX::XMVector3Equal(DirectX::XMLoadFloat3(&a), DirectX::XMVectorZero()))
		//cam->SetPosition(a);
		//ocam.Zoom(e.GetYOffset() / 100, (float)wndwith/(float)wndheight);
		
		return false;
	}
	~ExampleLayer() {
		delete shader;
		delete noreflect;
		delete envshader;
		delete roughnesst;
		delete normalt;
		//delete cam;
	}
private:
	Pistachio::RendererCBHandle cBuf1;
	Pistachio::RendererCBHandle cBuf2;
	Pistachio::Model mesh;
	Pistachio::Model* sphere;
	Pistachio::Model cube;
	Pistachio::Shader* shader;
	Pistachio::Shader* noreflect;
	RHI::DescriptorSet* set;
	bool vsync = true;
	Pistachio::Shader* envshader;
	Pistachio::RenderTexture rtx;
	float c[4] = {0.7f,0.7f,0.7f,0.7f};
	int wndwith = 0;
	int wndheight = 0;
	Pistachio::Ref<Pistachio::Texture2D> metalt;
	Pistachio::Ref<Pistachio::Texture2D> albedot;
	Pistachio::Texture2D* roughnesst;
	Pistachio::Texture2D* normalt;
	Pistachio::EditorCamera cam;
	Pistachio::ShaderBindingInfo info;
	Pistachio::Shader* shad;
	float color[4] = { 0.4f, 0.4f, 0.4f, 1.f };
	float metal = 0;
	float rough = 0;
	float ao = 1.f;
	float delta;
	Pistachio::OrthographicCamera ocam;
	struct ProfileResult
	{
		const char* Name;
		float Time;
	};
	std::vector<ProfileResult> profileResults;
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
