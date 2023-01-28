#define PISTACHIO_RENDER_API_DX11
#define IMGUI
#include "Pistachio.h"
#include "Pistachio/Core/Error.h"

#include "Pistachio/Core/EntryPoint.h"
class ExampleLayer : public Pistachio::Layer
{
public:
	ExampleLayer(const char* name) : Layer(name), cam(new Pistachio::PerspectiveCamera(45.0f, 0.1f, 100.0f)), mesh("sphere.obj"), cube("cube.obj"), ocam(-10.f, 10.f, 10.f, -10.f, 1.6)
	{
		albedot.reset(Pistachio::Texture2D::Create("Cerberus_A.tga"));
		metalt.reset(Pistachio::Texture2D::Create("Cerberus_M.tga"));
		roughnesst = Pistachio::Texture2D::Create("Cerberus_R.tga");
		normalt = Pistachio::Texture2D::Create("Cerberus_N.tga");
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
		PT_PROFILE_FUNCTION()
		Pistachio::RendererBase::ChangeViewport(rtx.GetWidth(), rtx.GetHeight());
		this->delta = delta;
		//Pistachio::RendererBase::SetPrimitiveTopology(Pistachio::PrimitiveTopology::TriangleList);
		cam->ChangeAspectRatio((float)wndwith / (float)wndheight);
		Pistachio::Renderer::BeginScene(cam, &rtx, 1);
		rtx.Clear(color);
		rtx.Bind();

		///*Default 3D scene
		//
		//
		//albedot->Bind(4);
		//roughnesst->Bind(5);
		//metalt->Bind(6);
		//normalt->Bind(7);
		//DirectX::XMMATRIX transform = DirectX::XMMatrixScaling(0.01f, 0.01f, 0.01f);
		//transform *= DirectX::XMMatrixRotationRollPitchYaw(DirectX::XMConvertToRadians(-90.f), DirectX::XMConvertToRadians(-180.f), DirectX::XMConvertToRadians(90.f));
		for (auto& mesh : mesh.meshes)
			Pistachio::Renderer::Submit(&mesh, shader, color, metal, rough, ao);
		//for (auto& mesh : mesh.meshes) {
		//	for (int row = 0; row < 4; ++row)
		//	{
		//		for (int col = 0; col < 4; ++col)
		//		{
		//			Pistachio::Renderer::Submit(&mesh, shader, color, (float)row / 4.f, (float)col / (float)4, ao, DirectX::XMMatrixTranslation((float)(col - (4.f / 2)) * 2.5f, (float)(row - (4.f
		//				/ 2)) * 2.5f, 2.0f));
		//		}
		//	}
		//}
		//Pistachio::RendererBase::SetCullMode(Pistachio::CullMode::Front);
		//DirectX::XMFLOAT3X3 view;
		//DirectX::XMStoreFloat3x3(&view, cam->GetViewMatrix());
		//Pistachio::Renderer::Submit(&cube.meshes[0], envshader, color, metal, rough, ao, DirectX::XMMatrixIdentity(), DirectX::XMMatrixTranspose(DirectX::XMLoadFloat3x3(&view) * cam->GetProjectionMatrix()));
		//Pistachio::RendererBase::SetCullMode(Pistachio::CullMode::Back);
		//Pistachio::Renderer2D::BeginScene(ocam);
		//static float positionx, positiony = 0, sizex =1, sizey = 1;
		/*for (int i = 0; i < 100; i++)
			for(int x = 0; x < 100; x++)
				Pistachio::Renderer2D::DrawQuad({ positionx + 2 ,positiony + 2 , 0}, {sizex,sizey}, {1.f,0,0,1.f});*/
		//ImGui::Text("%.3f", ImGui::GetIO().Framerate);
		//Pistachio::Renderer2D::EndScene();
	}
	void OnAttach() override
	{
	}
	void OnImGuiRender()
	{
		ImGui::DockSpaceOverViewport();
		ImGui::BeginMainMenuBar();
		if (ImGui::BeginMenu("File"))
		{
			if (ImGui::MenuItem("Exit"))
				PostMessageA(Pistachio::Application::Get().GetWindow().pd.hwnd, WM_CLOSE, 0, 0);
			ImGui::EndMenu();
		}
		ImGui::EndMainMenuBar();
		auto target = Pistachio::RendererBase::GetmainRenderTargetView();
		Pistachio::RendererBase::Getd3dDeviceContext()->OMSetRenderTargets(1, &target, Pistachio::RendererBase::GetDepthStencilView());
		auto a = cam->GetPosition();
		bool b = true;
		ImGui::ShowDemoWindow(&b);
		ImGui::Begin("lol");
		for (auto& result : profileResults)
		{
			char label[50];
			strcpy_s(label, "%.3fms ");
			strcat_s(label, result.Name);
			ImGui::Text(label, result.Time);
		}
		profileResults.clear();
		static bool popen = true;
		ImGui::Checkbox("Open", &popen);
		ImGui::ColorEdit4("Albedo", color);
		ImGui::SliderFloat("Metallic", &metal, 0, 1);
		ImGui::SliderFloat("Roughness", &rough, 0, 1);
		ImGui::SliderFloat("AO", &ao, 0, 1);
		if (ImGui::Button("Toggle Vsync", ImVec2(100*2.5, 20*2.5))) {
			vsync = 1 - vsync;
			Pistachio::Application::Get().GetWindow().SetVsync(vsync);
		}
		if (ImGui::Button("Catwalk.hdr", ImVec2(100*2.5, 20*2.5)))
			Pistachio::Renderer::Init("resources/textures/hdr/catwalk.hdr");
		else if (ImGui::Button("Newport Loft.hdr", ImVec2(100*2.5, 20*2.5)))
			Pistachio::Renderer::Init("resources/textures/hdr/newport_loft.hdr");
		else if (ImGui::Button("Empty Room.hdr", ImVec2(100*2.5, 20*2.5)))
			Pistachio::Renderer::Init("resources/textures/hdr/medium_res.hdr");
		else if (ImGui::Button("Apartment Reflect.hdr", ImVec2(100*2.5, 20*2.5)))
			Pistachio::Renderer::Init("resources/textures/hdr/Apartment_Reflection.hdr");
		else if (ImGui::Button("Alex Apartment.hdr", ImVec2(100 * 2.5, 20 * 2.5)))
			Pistachio::Renderer::Init("resources/textures/hdr/Alexs_Apt_2k.hdr");
		ImGui::End();
		ImGui::Begin("Scene", &popen, 0);
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
		wndwith = (INT)ImGui::GetContentRegionAvail().x;
		wndheight = (INT)ImGui::GetContentRegionAvail().y;
		cam->SetPosition(pos);
		ImGui::Image(rtx.GetSRV(), ImGui::GetContentRegionAvail());
		ImGui::End();

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
	Sandbox() { PushLayer(new ExampleLayer("lol")); GetWindow().SetVsync(0); }
	~Sandbox() { Pistachio::Renderer::Shutdown(); }
private:
};

Pistachio::Application* Pistachio::CreateApplication()
{
	return new Sandbox;
}
