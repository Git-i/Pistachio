#define PISTACHIO_RENDER_API_DX11
#include "Pistachio.h"
#include "Pistachio/Core/Error.h"

#include "Pistachio/Core/EntryPoint.h"
RHI::Viewport vp;

class ExampleLayer : public Pistachio::Layer
{
public:
	ExampleLayer(const char* name) : 
		Layer(name), cam(45.f, 16.f/9.f, 0.1, 100.f),
		mesh("sphere.obj"), 
		cube("cube.obj"), ocam(-10.f, 10.f, 10.f, -10.f, 1.6)
	{
	}
	void OnUpdate(float delta) override
	{
		PT_PROFILE_FUNCTION();
		RHI::Area2D area;
		area.offset = { 0,0 };
		area.size = {Pistachio::Application::Get().GetWindow().GetWidth(),Pistachio::Application::Get().GetWindow().GetHeight() };
		RHI::RenderingAttachmentDesc rdesc;
		cam.OnUpdate(delta);
		auto mat = cam.GetViewProjection();
		mat = mat.Transpose();
		cBuf->Update(&mat, sizeof(Pistachio::Matrix4), 0);
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
		Pistachio::RendererBase::GetMainCommandList()->BindDescriptorSet(shad->GetRootSignature(), info.sets[0].set, 0);
		mesh.meshes[0].GetVertexBuffer().Bind();
		mesh.meshes[0].GetIndexBuffer().Bind();
		Pistachio::RendererBase::GetMainCommandList()->DrawIndexed(mesh.meshes[0].GetIndexBuffer().GetCount(), 1, 0, 0, 0);
		cube.meshes[0].GetVertexBuffer().Bind();
		cube.meshes[0].GetIndexBuffer().Bind();
		//Pistachio::RendererBase::GetMainCommandList()->DrawIndexed(cube.meshes[0].GetIndexBuffer().GetCount(), 1, 0, 0, 0);

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

		info = {};
		shad = Shader::Create(&desc);
		RHI::DepthStencilMode currMode{};
		shad->CreateShaderBinding(info);
		//dsMode->DepthFunc = RHI::ComparisonFunc::Always;
		shad->SetDepthStencilMode(dsMode, ShaderModeSetFlags::AutomaticallyCreate);
		currMode.DepthEnable = true;
		currMode.DepthFunc = RHI::ComparisonFunc::LessEqual;
		currMode.DepthWriteMask = RHI::DepthWriteMask::All;
		currMode.StencilEnable = false;
		shad->SetDepthStencilMode(&currMode, ShaderModeSetFlags::AutomaticallyCreate);
		
		struct
		{
			Matrix4 viewProjection;
			Matrix4 transform = Matrix4(1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1);
			Vector4 viewPos = Vector4(0,10,-20,1);
			Vector4 albedo = Vector4(0.4,0,3.5,1);
			float metallic = 0.f;
			float roughness = 1.f;
			float ao = 1.f;
		}CBData;
		CBData.viewProjection = cam.GetViewProjection();
		cBuf = ConstantBuffer::Create(&CBData, sizeof(CBData));
		BufferBindingUpdateDesc bindingUpdate;
		bindingUpdate.buffer = cBuf->GetID();
		bindingUpdate.offset = 0;
		bindingUpdate.size = sizeof(CBData);
		info.UpdateBufferBinding(0, &bindingUpdate, 0);
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
	Pistachio::ConstantBuffer* cBuf;
	Pistachio::Model mesh;
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
