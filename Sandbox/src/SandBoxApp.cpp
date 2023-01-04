#define PISTACHIO_RENDER_API_DX11
#include "Pistachio.h"
#include "Pistachio/Core/Error.h"
using namespace DirectX;

class ExampleLayer : public Pistachio::Layer
{
public:
	ExampleLayer(const char* name) : Layer(name), cam(std::make_shared<Pistachio::PerspectiveCamera>(45.0f, 0.1f, 100.0f)), mesh("circle.obj"), cube("cube.obj")
	{
		albedot.reset(Pistachio::Texture2D::Create("Cerberus_A.tga"));
		metalt.reset(Pistachio::Texture2D::Create("Cerberus_M.tga"));
		roughnesst.reset(Pistachio::Texture2D::Create("Cerberus_R.tga"));
		normalt.reset(Pistachio::Texture2D::Create("Cerberus_N.tga"));
		samplerstate.reset(Pistachio::SamplerState::Create(Pistachio::TextureAddress::Wrap,Pistachio::TextureAddress::Wrap,Pistachio::TextureAddress::Wrap));
		rtx.CreateStack(1280, 720, 1, Pistachio::TextureFormat::RGBA8U);
		shader = new Pistachio::Shader(L"VertexShader.cso", L"PixelShader.cso");
		noreflect = new Pistachio::Shader(L"PBR_no_reflect_vs.cso", L"PBR_no_reflect_fs.cso");
		envshader = new Pistachio::Shader(L"background_vs.cso", L"background.cso");
		shader->CreateLayout(Pistachio::Mesh::GetLayout(), Pistachio::Mesh::GetLayoutSize());
		noreflect->CreateLayout(Pistachio::Mesh::GetLayout(), Pistachio::Mesh::GetLayoutSize());
		envshader->CreateLayout(Pistachio::Mesh::GetLayout(), Pistachio::Mesh::GetLayoutSize());
		cam->SetPosition(0, 0, 5);
	}
	void OnUpdate(float delta) override
	{
		Pistachio::RendererBase::ChangeViewport(rtx.GetWidth(), rtx.GetHeight());
		this->delta = delta;
		Pistachio::RendererBase::SetClearColor(0.6f, 0.6f, 0.6f, 1.0f);
		Pistachio::RendererBase::ClearView();
		Pistachio::RendererBase::SetPrimitiveTopology(Pistachio::PrimitiveTopology::TriangleList);
		cam->ChangeAspectRatio((float)wndwith / (float)wndheight);
		Pistachio::Renderer::BeginScene(cam.get(), &rtx);
		samplerstate->Bind();
		Pistachio::RendererBase::SetCullMode(Pistachio::CullMode::Front);
		DirectX::XMFLOAT3X3 view;
		DirectX::XMStoreFloat3x3(&view, cam->GetViewMatrix());
		Pistachio::Renderer::Submit(&cube.meshes[0], envshader, color, metal, rough, ao, DirectX::XMMatrixIdentity(), DirectX::XMMatrixTranspose(DirectX::XMLoadFloat3x3(&view) * cam->GetProjectionMatrix()));
		Pistachio::RendererBase::SetCullMode(Pistachio::CullMode::Back);
		albedot->Bind(4);
		roughnesst->Bind(5);
		metalt->Bind(6);
		normalt->Bind(7);
		//DirectX::XMMATRIX transform = DirectX::XMMatrixScaling(0.01, 0.01, 0.01);
		//transform *= DirectX::XMMatrixRotationRollPitchYaw(DirectX::XMConvertToRadians(-90.f), DirectX::XMConvertToRadians(-180.f), DirectX::XMConvertToRadians(90.f));
		//for (auto& mesh : mesh.meshes)
		//	Pistachio::Renderer::Submit(&mesh, shader, color, metal, rough, ao);
			for (auto& mesh : mesh.meshes) {
			for (int row = 0; row < 10; ++row)
			{
				for (int col = 0; col < 10; ++col)
				{
					Pistachio::Renderer::Submit(&mesh, shader, color, (float)row / 10.f, (float)col / (float)10, ao, DirectX::XMMatrixTranslation((float)(col - (10.f / 2)) * 2.5f, (float)(row - (10.f / 2)) * 2.5f, 2.0f));
				}
			}
		}
	}
	void OnAttach() override
	{
	}
	void OnImGuiRender()
	{
		ImGui::DockSpaceOverViewport();
		auto target = Pistachio::RendererBase::GetmainRenderTargetView();
		Pistachio::RendererBase::Getd3dDeviceContext()->OMSetRenderTargets(1, &target, Pistachio::RendererBase::GetDepthStencilView());
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
		if (ImGui::Button("Toggle Vsync", ImVec2(100, 20))) {
			vsync = 1 - vsync;
			Pistachio::Application::Get().GetWindow().SetVsync(vsync);
		}
		if (ImGui::Button("Catwalk.hdr", ImVec2(100, 20)))
			Pistachio::Renderer::Init("resources/textures/hdr/catwalk.hdr");
		else if (ImGui::Button("Newport Loft.hdr", ImVec2(100, 20)))
			Pistachio::Renderer::Init("resources/textures/hdr/newport_loft.hdr");
		else if (ImGui::Button("Empty Room.hdr", ImVec2(100, 20)))
			Pistachio::Renderer::Init("resources/textures/hdr/medium_res.hdr");
		else if (ImGui::Button("Apartment Reflection.hdr", ImVec2(100, 20)))
			Pistachio::Renderer::Init("resources/textures/hdr/Apartment_Reflection.hdr");
		ImGui::End();
		ImGui::Begin("Scene");
		auto camPos = cam->GetPosition();
		DirectX::XMFLOAT3 velocity;
		velocity.x = 0;
		velocity.y = 0;
		velocity.z = 0;

		if (ImGui::IsKeyDown(ImGuiKey_D) && ImGui::IsWindowFocused()) {
			velocity.x = 5.0;
		}
		else if (ImGui::IsKeyDown(ImGuiKey_A) && ImGui::IsWindowFocused()) {
			velocity.x = -5.0;
		}

		if (ImGui::IsKeyDown(ImGuiKey_W) && ImGui::IsWindowFocused())
			velocity.y = +5.0;
		else if (ImGui::IsKeyDown(ImGuiKey_S) && ImGui::IsWindowFocused())
			velocity.y = -5.0;

		DirectX::XMVector3Normalize(DirectX::XMLoadFloat3(&velocity));
		DirectX::XMFLOAT3 pos;
		pos.x = (camPos.x + velocity.x * delta);
		pos.y = (camPos.y + velocity.y * delta);
		pos.z = (camPos.z + velocity.z * delta);
		wndwith = ImGui::GetContentRegionAvail().x;
		wndheight = ImGui::GetContentRegionAvail().y;
		cam->SetPosition(pos);
		ImGui::Image(rtx.GetSRV(), ImGui::GetContentRegionAvail());
		ImGui::End();
		
	}
	void OnEvent(Pistachio::Event& event) override
	{
		Pistachio::EventDispatcher dispatcher(event);
		dispatcher.Dispatch<Pistachio::MouseScrolledEvent>(BIND_EVENT_FN(ExampleLayer::OnScroll));
		dispatcher.Dispatch<Pistachio::WindowResizeEvent>(BIND_EVENT_FN(ExampleLayer::OnResize));
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
		return false;
	}
	bool OnResize(Pistachio::WindowResizeEvent& e)
	{
		if (e.GetWidth())
		return false;
	}
	~ExampleLayer() {
		delete shader;
		delete noreflect;
		delete eqshader;
		delete envshader;
	}
private:
	Pistachio::Model mesh;
	Pistachio::Model cube;
	Pistachio::Shader* shader;
	Pistachio::Shader* noreflect;
	bool vsync = true;
	Pistachio::Shader* eqshader;
	Pistachio::Shader* envshader;
	Pistachio::RenderTexture rtx;
	float c[4] = {0.7f,0.7f,0.7f,0.7f};
	int wndwith = 0;
	int wndheight = 0;
	Pistachio::Ref<Pistachio::Texture2D> metalt;
	Pistachio::Ref<Pistachio::Texture2D> albedot;
	Pistachio::Ref<Pistachio::Texture2D> roughnesst;
	Pistachio::Ref<Pistachio::Texture2D> normalt;
	Pistachio::Ref<Pistachio::SamplerState> samplerstate;
	Pistachio::Ref<Pistachio::PerspectiveCamera> cam;
	float color[3] = { 1.f, 0.f, 0.f };
	float metal = 0;
	float rough = 0;
	float ao = 1.f;
	float delta;
};
class Sandbox : public Pistachio::Application
{
public:
	Sandbox() { PushLayer(new ExampleLayer("lol")); GetWindow().SetVsync(0); }
	~Sandbox() { Pistachio::Renderer::Shutdown(); }
private:
};

Pistachio::Application* Pistachio::CreateApplication()
{
	return new Sandbox;
}