#define PISTACHIO_RENDER_API_DX11
#include "Pistachio.h"

struct Vertex
{
	float x, y, u, v;
};
Vertex vertices[6]
{
	{ 0.0f,  0.5f, 1.0f, 0.0f},
	{ 0.5f, -0.5f, 1.0f, 0.0f},
	{-0.5f, -0.5f, 1.0f, 0.0f},
	{-0.3f,  0.3f, 1.0f, 0.0f},
	{ 0.3f,  0.3f, 1.0f, 0.0f},
	{ 0.0f, -0.8f, 1.0f, 0.0f}
};
const unsigned short indices[]
{
	0, 1, 2,
	0, 2, 3,
	0, 4, 1,
	2, 1, 5
};
Vertex rectVertives[]
{
	{-0.5,  0.5, 0, 0/*-0.25, -0.25*/},
	{ 0.5,  0.5, 1, 0/*1.25, -0.25*/},
	{ 0.5, -0.5, 1, 1/*1.25, 1.25*/},
	{-0.5, -0.5, 0, 1/*-0.25, 1.25*/}
};
const unsigned short rectIndices[]
{
	0, 1, 2,
	2, 3, 0
};
class ExampleLayer : public Pistachio::Layer
{
public:
	ExampleLayer(const char* name) : Layer(name), BasicShader(std::make_shared<Pistachio::Shader>(L"VertexShader.cso", L"PixelShader.cso")), cam(std::make_shared<Pistachio::PerspectiveCamera>(45.0f, 0.1f, 100.0f, 16.0f / 9.0f))
	{
		vertexbuffer.reset(Pistachio::VertexBuffer::Create(vertices, sizeof(vertices), sizeof(Vertex)));
		indexbuffer.reset(Pistachio::IndexBuffer::Create(indices, sizeof(indices), sizeof(unsigned short)));
		texture.reset(Pistachio::Texture2D::Create("texture.png"));
		rectVertexBuffer.reset(Pistachio::VertexBuffer::Create(rectVertives, sizeof(rectVertives), sizeof(Vertex)));
		rectIndexbuffer.reset(Pistachio::IndexBuffer::Create(indices, sizeof(rectIndices), sizeof(unsigned short)));
		samplerstate.reset(Pistachio::SamplerState::Create());
		hexagonBuffer = std::make_shared<Pistachio::Buffer>(vertexbuffer.get(), indexbuffer.get());
		rectBuffer = std::make_shared<Pistachio::Buffer>(rectVertexBuffer.get(), rectIndexbuffer.get());
		Pistachio::BufferLayout layout[] = {
		{"POSITION", Pistachio::BufferLayoutFormat::FLOAT2, 0},
		{"UV", Pistachio::BufferLayoutFormat::FLOAT2, 8}
		};

		BasicShader->CreateLayout(layout, 2);
	}
	void OnUpdate(float delta) override
	{
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
		cam->SetPosition(pos);
		auto f = Pistachio::RendererBase::Getd3dDeviceContext()->GetType();
		Pistachio::Renderer::BeginScene(cam.get());
		Pistachio::RendererBase::SetClearColor(0.6f, 0.6f, 0.6f, 1.0f);
		Pistachio::RendererBase::ClearView();
		samplerstate->Bind();
		texture->Bind();
		//Pistachio::Renderer::Submit(hexagonBuffer, BasicShader);
		Pistachio::Renderer::Submit(rectBuffer.get(), BasicShader.get());
	}
	void OnAttach() override
	{
	}
	void OnImGuiRender()
	{
		auto a = cam->GetPosition();
		ImGui::Begin("lol");
		ImGui::Text(std::to_string(a.x).c_str());
		static int y[2] = { 0, 0 };
		ImGui::SliderInt2("Controller vibration", y, 0, 65000);
		Pistachio::Input::VibrateController(1, y[0], y[1]);
		ImGui::Text(std::to_string(a.y).c_str());
		ImGui::Text(std::to_string(a.z).c_str());
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
	Pistachio::Ref<Pistachio::Texture2D> texture;
	Pistachio::Ref<Pistachio::SamplerState> samplerstate;
	Pistachio::Ref<Pistachio::Shader> BasicShader;
	Pistachio::Ref<Pistachio::PerspectiveCamera> cam;
	Pistachio::Ref<Pistachio::VertexBuffer> vertexbuffer;
	Pistachio::Ref<Pistachio::IndexBuffer> indexbuffer;
	Pistachio::Ref<Pistachio::VertexBuffer> rectVertexBuffer;
	Pistachio::Ref<Pistachio::IndexBuffer> rectIndexbuffer;
	Pistachio::Ref<Pistachio::Buffer> hexagonBuffer;
	Pistachio::Ref<Pistachio::Buffer> rectBuffer;
	
};
class Sandbox : public Pistachio::Application
{
public:
	Sandbox() { PushLayer(new ExampleLayer("velocity")); GetWindow().SetVsync(1); }
	~Sandbox(){}
};

Pistachio::Application* Pistachio::CreateApplication()
{
	return new Sandbox;
}