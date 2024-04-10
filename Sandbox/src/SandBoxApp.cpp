#define PISTACHIO_RENDER_API_DX11
#include "Pistachio.h"
#include "Pistachio/Core/Error.h"

#include "Pistachio/Core/EntryPoint.h"
using namespace Pistachio;
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
struct BackgroundCBStruct
{
	Pistachio::Matrix4 pad[4];
	Pistachio::Matrix4 viewProj;
	Pistachio::Matrix4 invViewProj;
}BackgroundCB;
class ExampleLayer : public Pistachio::Layer
{
public:
	ExampleLayer(const char* name) :
		Layer(name), cam(Math::ToRadians(45.f), 16.f/9.f, .1f,100.f)

	{
	}
	void OnUpdate(float delta) override
	{
		scene.OnUpdateEditor(delta, cam);

		
	}
	void OnAttach() override
	{
		Entity e = scene.CreateEntity("Mesh");
		auto& mrc = e.AddComponent<MeshRendererComponent>("circle.obj");
		Material* mat = new Material();
		mat->SetShader(GetAssetManager()->CreateShaderAsset("Default Shader"));
		Shader* shader = GetAssetManager()->GetShaderResource(mat->shader)->GetShader();
		shader->GetPSShaderBinding(mat->mtlInfo, 3);
		mat->mtlInfo.UpdateTextureBinding(Renderer::GetWhiteTexture().GetView(), 0);
		mat->mtlInfo.UpdateTextureBinding(Renderer::GetWhiteTexture().GetView(), 1);
		mat->mtlInfo.UpdateTextureBinding(Renderer::GetWhiteTexture().GetView(), 2);
		mat->mtlInfo.UpdateTextureBinding(Renderer::GetWhiteTexture().GetView(), 3);
		mrc.modelIndex = 0;
		mrc.material = GetAssetManager()->FromResource(mat, "Matrrl", ResourceType::Material);
		mrc.handle = Renderer::AllocateConstantBuffer(sizeof(DirectX::XMFLOAT4X4) * 2);

	}
	void OnImGuiRender()
	{
		

	}
	~ExampleLayer() {
	}
private:
	Scene scene;
	EditorCamera cam;
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
