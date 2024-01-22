#define PISTACHIO_RENDER_API_DX11
#include "Pistachio.h"
#include "Pistachio/Core/Error.h"

#include "Pistachio/Core/EntryPoint.h"
class ExampleLayer : public Pistachio::Layer
{
public:
	ExampleLayer(const char* name) : Layer(name), cam(new Pistachio::PerspectiveCamera(45.0f, 0.1f, 100.0f)), mesh("sphere.obj"), cube("cube.obj"), ocam(-10.f, 10.f, 10.f, -10.f, 1.6)
	{
	}
	void OnUpdate(float delta) override
	{
		PT_PROFILE_FUNCTION();
		
		
	}
	void OnAttach() override
	{
		RHI::DepthStencilMode dsMode[2]{};
		dsMode[0].DepthEnable = false;
		dsMode[0].StencilEnable = false;
		dsMode[0].DepthFunc = RHI::ComparisonFunc::LessEqual;
		dsMode[0].DepthWriteMask = RHI::DepthWriteMask::All;
		dsMode[1].DepthEnable = true;
		dsMode[1].StencilEnable = false;
		dsMode[1].DepthFunc = RHI::ComparisonFunc::LessEqual;
		dsMode[1].DepthWriteMask = RHI::DepthWriteMask::All;
		RHI::BlendMode blendMode{};
		blendMode.IndependentBlend = true;
		blendMode.blendDescs[0].blendEnable = false;
		RHI::RasterizerMode rsMode{};
		rsMode.fillMode = RHI::FillMode::Solid;
		rsMode.AntialiasedLineEnable = false;
		rsMode.conservativeRaster = false;
		rsMode.cullMode = RHI::CullMode::Back;
		rsMode.depthBias = 0;
		rsMode.multisampleEnable = false;
		using namespace Pistachio;
		BufferLayout layout("SEM0", BufferLayoutFormat::FLOAT4, 0);
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
		desc.numInputs = 1;
		desc.InputDescription = &layout;
		desc.BlendModes = &blendMode;
		Shader* shad = Shader::Create(&desc);
		Pistachio::ShaderBindingInfo info;
		shad->CreateShaderBinding(info);
		int a = 9;
	}
	void OnImGuiRender()
	{
		

	}
	void OnEvent(Pistachio::Event& event) override
	{
		Pistachio::EventDispatcher dispatcher(event);
		dispatcher.Dispatch<Pistachio::MouseScrolledEvent>(BIND_EVENT_FN(ExampleLayer::OnScroll));
	}
	bool OnScroll(Pistachio::MouseScrolledEvent& e)
	{
		auto a = cam->GetPosition();
		DirectX::XMFLOAT3 dir;
		DirectX::XMStoreFloat3(&dir,DirectX::XMVectorSubtract(DirectX::XMVectorZero(), DirectX::XMLoadFloat3(&a)));
		DirectX::XMStoreFloat3(&dir,DirectX::XMVector3Normalize(DirectX::XMLoadFloat3(&dir)));
		DirectX::XMStoreFloat3(&dir, DirectX::XMVectorScale(DirectX::XMLoadFloat3(&dir), e.GetYOffset() / 100));
		DirectX::XMStoreFloat3(&a, DirectX::XMVectorAdd(DirectX::XMLoadFloat3(&a), DirectX::XMLoadFloat3(&dir)));
		if(!DirectX::XMVector3Equal(DirectX::XMLoadFloat3(&a), DirectX::XMVectorZero()))
		cam->SetPosition(a);
		ocam.Zoom(e.GetYOffset() / 100, (float)wndwith/(float)wndheight);
		return false;
	}
	~ExampleLayer() {
		delete shader;
		delete noreflect;
		delete envshader;
		delete roughnesst;
		delete normalt;
		delete cam;
	}
private:
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
	Pistachio::PerspectiveCamera* cam;
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
