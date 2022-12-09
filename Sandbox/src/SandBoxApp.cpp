#define PISTACHIO_RENDER_API_DX11
#include "Pistachio.h"

struct Vertex
{
	float x, y, r, g, b;
};
Vertex vertices[6]
{
	{ 0.0f,  0.5f, 1.0f, 0,    0.19f},
	{ 0.5f, -0.5f, 1.0f, 0,    1.0f },
	{-0.5f, -0.5f, 1.0f, 0,    1.0f },
	{-0.3f,  0.3f, 1.0f, 0,    0.19f},
	{ 0.3f,  0.3f, 1.0f, 0.0f, 0.19f},
	{ 0.0f, -0.8f, 1.0f, 0,    1.0f }
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
	{-0.5, 0.5, 1.0, 1.0, 1.0},
	{0.5, 0.5, 1.0, 1.0, 1.0},
	{0.5, -0.5, 1.0, 1.0, 1.0},
	{-0.5, -0.5, 1.0, 1.0, 1.0}
};
const unsigned short rectIndices[]
{
	0, 1, 2,
	2, 3, 0
};
class ExampleLayer : public Pistachio::Layer
{
public:
	ExampleLayer(const char* name) : Layer(name), BasicShader(L"VertexShader.cso", L"PixelShader.cso"), cam(-1.6f, 1.6f, 0.9f, -0.9f)
	{
		vertexbuffer.Initialize(vertices, sizeof(vertices), sizeof(Vertex));
		indexbuffer.Initialize(indices, sizeof(indices), sizeof(unsigned short));

		rectVertexBuffer.Initialize(rectVertives, sizeof(rectVertives), sizeof(Vertex));
		rectIndexbuffer.Initialize(indices, sizeof(rectIndices), sizeof(unsigned short));

		Pistachio::BufferLayout layout[] = {
		{"POSITION", Pistachio::BufferLayoutFormat::FLOAT2, 0},
		{"COLOR", Pistachio::BufferLayoutFormat::FLOAT3, 8}
		};

		BasicShader.CreateLayout(layout, 2);
	}
	void OnUpdate(float delta) override
	{
		auto a = cam.GetPosition();
		DirectX::XMFLOAT3 velocity;
		velocity.x = 0;
		velocity.y = 0;
		velocity.z = 0;
		if (Pistachio::Input::IsKeyPressed(PT_KEY_D))
			velocity.x = -5.0;
		else if (Pistachio::Input::IsKeyPressed(PT_KEY_A))
			velocity.x = 5.0;

		if (Pistachio::Input::IsKeyPressed(PT_KEY_W))
			velocity.y = -5.0;
		else if (Pistachio::Input::IsKeyPressed(PT_KEY_S))
			velocity.y = 5.0;
		DirectX::XMVector3Normalize(DirectX::XMLoadFloat3(&velocity));
		DirectX::XMFLOAT3 pos;
		pos.x = (a.x + velocity.x * delta);
		pos.y = (a.y + velocity.y * delta);
		pos.z = (a.z + velocity.z * delta);
		cam.SetPosition(pos);
		Pistachio::Renderer::BeginScene(cam);
		
		Pistachio::RendererBase::SetClearColor(1.6f, 0.6f, 0.6f, 1.0f);
		Pistachio::RendererBase::ClearView();

		Pistachio::Renderer::Submit(rectBuffer, BasicShader);
		Pistachio::Renderer::Submit(hexagonBuffer, BasicShader);
	}
	void OnAttach() override
	{
		PT_INFO("lol");
	}
	void OnEvent(Pistachio::Event& event) override
	{
		Pistachio::EventDispatcher dispatcher(event);
		dispatcher.Dispatch<Pistachio::KeyPressedEvent>(BIND_EVENT_FN(ExampleLayer::OnKeyPressed));
	}
	bool OnKeyPressed(Pistachio::KeyPressedEvent& e)
	{
		
		return false;
	}
private:
	Pistachio::Shader BasicShader;
	Pistachio::OrthographicCamera cam;
	Pistachio::VertexBuffer vertexbuffer;
	Pistachio::IndexBuffer indexbuffer;
	Pistachio::VertexBuffer rectVertexBuffer;
	Pistachio::IndexBuffer rectIndexbuffer;
	Pistachio::Buffer hexagonBuffer = { &vertexbuffer, &indexbuffer };;
	Pistachio::Buffer rectBuffer = { &rectVertexBuffer, &rectIndexbuffer };
	
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