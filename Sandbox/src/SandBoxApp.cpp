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
		CBData2.transform = Pistachio::Matrix4::CreateTranslation(sinf((float)frame / 50.f)*3, 0, 0).Transpose();
		Pistachio::Renderer::FullCBUpdate(cBuf1, &CBData);
		Pistachio::Renderer::FullCBUpdate(cBuf2, &CBData2);
		rdesc.clearColor = { 0.34,0.34,0.34,0.34 };
		rdesc.ImageView.val = Pistachio::RendererBase::GetRTVDescriptorHeap()->GetCpuHandle().val + (Pistachio::RendererBase::GetCurrentRTVIndex() * Pistachio::RendererBase::Getd3dDevice()->GetDescriptorHeapIncrementSize(RHI::DescriptorClass::RTV));
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
		Pistachio::RendererBase::GetMainCommandList()->DrawIndexed(mesh.meshes[0].GetIBHandle().size/sizeof(uint32_t),
			1,
			Pistachio::Renderer::GetIBOffset(mesh.meshes[0].GetIBHandle()),
			Pistachio::Renderer::GetVBOffset(mesh.meshes[0].GetVBHandle()), 0);
		Pistachio::RendererBase::GetMainCommandList()->BindDynamicDescriptor(shad->GetRootSignature(), Pistachio::Renderer::GetCBDesc(), 0, Pistachio::Renderer::GetCBOffset(cBuf2));
		if (frame < 3000 || frame > 5000)
		{
			Pistachio::RendererBase::GetMainCommandList()->DrawIndexed(sphere->meshes[0].GetIBHandle().size / sizeof(uint32_t),
				1,
				Pistachio::Renderer::GetIBOffset(sphere->meshes[0].GetIBHandle()) / sizeof(uint32_t),
				Pistachio::Renderer::GetVBOffset(sphere->meshes[0].GetVBHandle()) / sizeof(Pistachio::Vertex), 0);
		}
		else
		{
			if (frame == 3002)
				delete sphere;
			if (frame == 3004)
				sphere = new Pistachio::Model("circle.obj");
			Pistachio::RendererBase::GetMainCommandList()->DrawIndexed(cube.meshes[0].GetIBHandle().size / sizeof(uint32_t),
				1,
				Pistachio::Renderer::GetIBOffset(cube.meshes[0].GetIBHandle()) / sizeof(uint32_t),
				Pistachio::Renderer::GetVBOffset(cube.meshes[0].GetVBHandle()) / sizeof(Pistachio::Vertex), 0);
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

		RHI::RootParameterDesc rpDesc;
		rpDesc.type = RHI::RootParameterType::DynamicDescriptor;
		rpDesc.dynamicDescriptor.stage = RHI::ShaderStage::Vertex;
		rpDesc.dynamicDescriptor.type = RHI::DescriptorType::ConstantBufferDynamic;
		RHI::RootSignatureDesc rsDesc;
		rsDesc.numRootParameters = 1;
		rsDesc.rootParameters = &rpDesc;
		RHI::RootSignature* rs;
		RendererBase::Getd3dDevice()->CreateRootSignature(&rsDesc, &rs, nullptr);
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
		
		BufferBindingUpdateDesc bindingUpdate;
		bindingUpdate.buffer = NULL;
		bindingUpdate.offset = 0;
		bindingUpdate.size = sizeof(CBData);
		bindingUpdate.type = RHI::DescriptorType::ConstantBuffer;
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
