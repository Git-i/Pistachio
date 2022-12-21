#define PISTACHIO_RENDER_API_DX11
#include "Pistachio.h"
#include "Pistachio/Renderer/DirectX11/DX11Texture.h"
using namespace Pistachio;
using namespace DirectX;

class ExampleLayer : public Pistachio::Layer
{
public:
	ExampleLayer(const char* name) : Layer(name),  cam(std::make_shared<Pistachio::PerspectiveCamera>(45.0f, 0.1f, 100.0f, 16.0f / 9.0f))
	{
		samplerstate.reset(Pistachio::SamplerState::Create());
		texture.reset(Pistachio::Texture2D::Create("brdf.png"));
		rtx.CreateStack(1280, 720);
		shader = new Shader(L"VertexShader.cso", L"PixelShader.cso");
		noreflect = new Shader(L"PBR_no_reflect_vs.cso", L"PBR_no_reflect_fs.cso");
		envshader = new Shader(L"background_vs.cso", L"background.cso");
		shader->CreateLayout(Pistachio::Mesh::GetLayout(), Pistachio::Mesh::GetLayoutSize());
		noreflect->CreateLayout(Pistachio::Mesh::GetLayout(), Pistachio::Mesh::GetLayoutSize());
		envshader->CreateLayout(Pistachio::Mesh::GetLayout(), Pistachio::Mesh::GetLayoutSize());
		cam->SetPosition(0, 0, 5);
		mesh = Mesh::Create("sphere.obj");
		cube.CreateStack("cube.obj");
		plane.CreateStack("plane.obj");
		
	}
	void OnUpdate(float delta) override
	{
		rtx.Clear(c);
		rtx.Bind();
		auto a = cam->GetPosition();
		DirectX::XMFLOAT3 velocity;
		velocity.x = 0;
		velocity.y = 0;
		velocity.z = 0;
		if (Pistachio::Input::IsKeyPressed(PT_KEY_D))
			velocity.x = +5.0;
		else if (Pistachio::Input::IsKeyPressed(PT_KEY_A))
			velocity.x = -5.0;

		if (Pistachio::Input::IsKeyPressed(PT_KEY_W))
			velocity.y = +5.0;
		else if (Pistachio::Input::IsKeyPressed(PT_KEY_S))
			velocity.y = -5.0;
		DirectX::XMVector3Normalize(DirectX::XMLoadFloat3(&velocity));
		DirectX::XMFLOAT3 pos;
		pos.x = (a.x + velocity.x * delta);
		pos.y = (a.y + velocity.y * delta);
		pos.z = (a.z);
		Pistachio::RendererBase::SetClearColor(0.6f, 0.6f, 0.6f, 1.0f);
		Pistachio::RendererBase::ClearView();
		RendererBase::SetPrimitiveTopology(PrimitiveTopology::TriangleList);
		cam->SetPosition(pos);
		Pistachio::Renderer::BeginScene(cam.get());
		samplerstate->Bind();
		DX11Texture::Bind(Renderer::GetFrambufferSRV(), 1);
		DirectX::XMFLOAT3X3 view;
		DirectX::XMStoreFloat3x3(&view, cam->GetViewMatrix());
		Renderer::Submit(&cube, envshader, color, metal, rough, ao, false, DirectX::XMMatrixIdentity(), DirectX::XMMatrixTranspose(DirectX::XMLoadFloat3x3(&view)*cam->GetProjectionMatrix()));
		texture->Bind(0);
		DX11Texture::Bind(Renderer::GetBrdfSRV(), 0);
		DX11Texture::Bind(Renderer::GetIrradianceFrambufferSRV(), 1);
		DX11Texture::Bind(Renderer::GetPrefilterFrambufferSRV(0), 2);
		if(reflect)
		Renderer::Submit(mesh, shader, color, metal, rough, ao, false);
		else
		Renderer::Submit(mesh, noreflect, color, metal, rough, ao, false);
	}
	void OnAttach() override
	{
	}
	void OnImGuiRender()
	{
		auto target = RendererBase::GetmainRenderTargetView();
		RendererBase::Getd3dDeviceContext()->OMSetRenderTargets(1, &target, RendererBase::GetDepthStencilView());
		auto a = cam->GetPosition();
		bool b = true;
		ImGui::ShowDemoWindow(&b);
		ImGui::Begin("lol");
		ImGui::Text(std::to_string(a.x).c_str());
		static int y[2] = { 0, 0 };
		ImGui::SliderInt2("Controller vibration", y, 0, 65000);
		Pistachio::Input::VibrateController(1, y[0], y[1]);
		ImGui::Text((std::string("Left Analog X: ") + std::to_string(Pistachio::Input::GetLeftAnalogX(0))).c_str());
		ImGui::Text((std::string("Left Analog Y: ") + std::to_string(Pistachio::Input::GetLeftAnalogY(0))).c_str());
		ImGui::Text((std::string("Right Analog X: ") + std::to_string(Pistachio::Input::GetRightAnalogX(0))).c_str());
		ImGui::Text((std::string("Right Analog Y: ") + std::to_string(Pistachio::Input::GetRightAnalogY(0))).c_str());
		ImGui::Text((std::string("Button A: ") + std::to_string(Pistachio::Input::IsGamepadButtonPressed(0, PT_GAMEPAD_A))).c_str());
		ImGui::Text((std::string("Button B: ") + std::to_string(Pistachio::Input::IsGamepadButtonPressed(0, PT_GAMEPAD_B))).c_str());
		ImGui::Text((std::string("Button X: ") + std::to_string(Pistachio::Input::IsGamepadButtonPressed(0, PT_GAMEPAD_X))).c_str());
		ImGui::Text((std::string("Button Y: ") + std::to_string(Pistachio::Input::IsGamepadButtonPressed(0, PT_GAMEPAD_Y))).c_str());
		ImGui::Text((std::string("Button Start: ") + std::to_string(Pistachio::Input::IsGamepadButtonPressed(0, PT_GAMEPAD_START))).c_str());
		ImGui::Text((std::string("Button Back: ") + std::to_string(Pistachio::Input::IsGamepadButtonPressed(0, PT_GAMEPAD_BACK))).c_str());
		ImGui::Text((std::string("Dpad Up: ") + std::to_string(Pistachio::Input::IsGamepadButtonPressed(0, PT_GAMEPAD_DPAD_UP))).c_str());
		ImGui::Text((std::string("Dpad Down: ") + std::to_string(Pistachio::Input::IsGamepadButtonPressed(0, PT_GAMEPAD_DPAD_DOWN))).c_str());
		ImGui::Text((std::string("Dpad Left: ") + std::to_string(Pistachio::Input::IsGamepadButtonPressed(0, PT_GAMEPAD_DPAD_LEFT))).c_str());
		ImGui::Text((std::string("Dpad Right: ") + std::to_string(Pistachio::Input::IsGamepadButtonPressed(0, PT_GAMEPAD_DPAD_RIGHT))).c_str());
		ImGui::Text((std::string("Button LB: ") + std::to_string(Pistachio::Input::IsGamepadButtonPressed(0, PT_GAMEPAD_LEFT_SHOULDER))).c_str());
		ImGui::Text((std::string("Button RB: ") + std::to_string(Pistachio::Input::IsGamepadButtonPressed(0, PT_GAMEPAD_RIGHT_SHOULDER))).c_str());
		ImGui::Text((std::string("Left Trigger: ") + std::to_string(Pistachio::Input::GetLeftTriggerState(0))).c_str());
		ImGui::Text((std::string("Right Trigger: ") + std::to_string(Pistachio::Input::GetRightTriggerState(0))).c_str());
		ImGui::Text((std::string("Button LT: ") + std::to_string(Pistachio::Input::IsGamepadButtonPressed(0, PT_GAMEPAD_LEFT_THUMB))).c_str());
		ImGui::Text((std::string("Button RT: ") + std::to_string(Pistachio::Input::IsGamepadButtonPressed(0, PT_GAMEPAD_RIGHT_THUMB))).c_str());
		ImGui::Text(std::to_string(a.z).c_str());
		
		ImGui::ColorEdit3("Albedo", color);
		ImGui::SliderFloat("Metallic", &metal, 0, 1);
		ImGui::SliderFloat("Roughness", &rough, 0, 1);
		ImGui::SliderFloat("AO", &ao, 0, 1);
		ImGui::Checkbox("Reflections: ", &reflect);
		ImGui::End();
		ImGui::Begin("Scene");
		ImGui::Image(rtx.shaderResourceView, ImVec2(640, 360));
		ImGui::End();
	}
	void OnEvent(Pistachio::Event& event) override
	{
		Pistachio::EventDispatcher dispatcher(event);
		dispatcher.Dispatch<Pistachio::MouseScrolledEvent>(BIND_EVENT_FN(ExampleLayer::OnKeyPressed));
	}
	bool OnKeyPressed(Pistachio::MouseScrolledEvent& e)
	{
		auto a = cam->GetPosition();
		DirectX::XMFLOAT3 dir;
		DirectX::XMStoreFloat3(&dir,DirectX::XMVectorSubtract(DirectX::XMVectorZero(), DirectX::XMLoadFloat3(&a)));
		DirectX::XMStoreFloat3(&dir,DirectX::XMVector3Normalize(DirectX::XMLoadFloat3(&dir)));
		DirectX::XMStoreFloat3(&dir, DirectX::XMVectorScale(DirectX::XMLoadFloat3(&dir), e.GetYOffset() / 100));
		DirectX::XMStoreFloat3(&a, DirectX::XMVectorAdd(DirectX::XMLoadFloat3(&a), DirectX::XMLoadFloat3(&dir)));
		if(!DirectX::XMVector3Equal(DirectX::XMLoadFloat3(&a), DirectX::XMVectorZero()))
		cam->SetPosition(a);
		return false;
	}
private:
	Mesh* mesh;
	Mesh cube;
	Mesh plane;
	Shader* shader;
	Shader* noreflect;
	bool reflect;
	Shader* eqshader;
	Shader* envshader;
	Texture3D* tex;
	RenderTexture rtx;
	float c[4] = {0.7f,0.7f,0.7f,0.7f};
	Pistachio::Ref<Pistachio::Texture2D> texture;
	Pistachio::Ref<Pistachio::SamplerState> samplerstate;
	Pistachio::Ref<Pistachio::PerspectiveCamera> cam;
	float color[3] = { 1.f, 0.f, 0.f };
	float metal = 0;
	float rough = 0;
	float ao = 1.f;
	
};
class Sandbox : public Pistachio::Application
{
public:
	Sandbox() { PushLayer(new ExampleLayer("velocity")); GetWindow().SetVsync(0); }
	~Sandbox(){}
};

Pistachio::Application* Pistachio::CreateApplication()
{
	return new Sandbox;
}