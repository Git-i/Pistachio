#define PISTACHIO_RENDER_API_DX11
#include "Pistachio.h"
#include "Pistachio/Core/Error.h"
#define PT_CUSTOM_ENTRY
#include "Pistachio/Core/EntryPoint.h"
#include "renderdoc_app.h"
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
	void OnEvent(Event& event) override
	{
		cam.OnEvent(event);
	}
	ExampleLayer(const char* name) :
		Layer(name), cam(45.f, 16.f/9.f, .1f,100.f)

	{
	}
	void OnUpdate(float delta) override
	{
		static int count = 0;
		count++;
		static float metallic = 1.f;
		static float dir = -1.f;
		static float time = 0.f;
		time += delta;
		mat->ChangeParam("Roughness", metallic);
		//metallic += delta * dir;
		if (metallic <= 0) dir = 1.f;
		if (metallic >= 1) dir = -1.f;
		//cmp->Translation.y = 10.f;//sinf(time) * 5.f;
		
		cam.OnUpdate(delta);
		scene.OnUpdateEditor(delta, cam);
		//Scene* scenes[] = {&scene, &scene,&scene,&scene};
		//FrameComposer::Compose(scenes, 4);
		
	}
	void OnAttach() override
	{
		Entity e = scene.CreateEntity("Mesh");
		auto& mrc = e.AddComponent<MeshRendererComponent>("torus.obj");
		e = scene.CreateEntity("Mesh2");
		auto& mrc2 = e.AddComponent<MeshRendererComponent>("cube.obj");
		auto& tc = e.GetComponent<TransformComponent>();
		e = scene.CreateEntity("Light");
		auto& lc = e.AddComponent<LightComponent>();
		auto& tcc = e.GetComponent<TransformComponent>();
		lc.color = { 1,1,0 };
		lc.exData.x = DirectX::XMScalarCos(Math::ToRadians(55.f/2.f));
		lc.exData.y = DirectX::XMScalarCos(Math::ToRadians(40.f/2.f));
		lc.exData.z = 15.f;
		lc.Intensity = 20.f;
		lc.shadow = true;
		lc.Type = LightType::Spot;
		tcc.Translation.z = -3.3;
		tcc.Translation.y = 2.0;
		tcc.RotationEulerHint.x = Math::ToRadians(40.f);
		//tcc.RotationEulerHint.y = Math::ToRadians(180.f);
		tcc.RecalculateRotation();
		e = scene.CreateEntity("SndLight");
		auto& clc = e.AddComponent<LightComponent>();
		auto& ctcc = e.GetComponent<TransformComponent>();
		clc.color = { 0,0,1 };
		clc.exData.x = DirectX::XMScalarCos(Math::ToRadians(55.f / 2.f));
		clc.exData.y = DirectX::XMScalarCos(Math::ToRadians(40.f / 2.f));
		clc.exData.z = 15.f;
		clc.Intensity = 20.f;
		clc.shadow = true;
		clc.Type = LightType::Spot;
		ctcc.Translation.z = -3.3;
		ctcc.Translation.y = -2.0;
		ctcc.RotationEulerHint.x = Math::ToRadians(-40.f);
		ctcc.RecalculateRotation();
		FrameComposer::SetCompositionMode(CompositionMode::SimpleCopy);
		
		cmp = &tc;
		cmp->Translation = Vector3{ 0.f,0.f,2.f };
		cmp->Scale.z = 1.f;
		cmp->Scale.x = 3.f;
		cmp->Scale.y = 3.f;
		mat = new Material();
		mat->SetShader(GetAssetManager()->CreateShaderAsset("Default Shader"));
		ShaderAsset* asset = GetAssetManager()->GetShaderResource(mat->shader);
		mat->parametersBuffer = Renderer::AllocateConstantBuffer(asset->GetParamBufferSize());
		mat->parametersBufferCPU = malloc(asset->GetParamBufferSize());
		mat->ChangeParam("Diffuse", 1);
		
		mat->ChangeParam("Metallic", .0f);
		Shader* shader = asset->GetShader();
		shader->GetPSShaderBinding(mat->mtlInfo, 3);
		mat->mtlInfo.UpdateTextureBinding(Renderer::GetWhiteTexture().GetView(), 0);
		mat->mtlInfo.UpdateTextureBinding(Renderer::GetWhiteTexture().GetView(), 1);
		mat->mtlInfo.UpdateTextureBinding(Renderer::GetWhiteTexture().GetView(), 2);
		mat->mtlInfo.UpdateTextureBinding(Renderer::GetWhiteTexture().GetView(), 3);
		mrc.modelIndex = 0;
		mrc.material = GetAssetManager()->FromResource(mat, "Matrrl", ResourceType::Material);
		mrc.handle = Renderer::AllocateConstantBuffer(sizeof(DirectX::XMFLOAT4X4) * 2);
		mrc2.modelIndex = 0;
		mrc2.material = GetAssetManager()->CreateMaterialAsset("Matrrl");
		mrc2.handle = Renderer::AllocateConstantBuffer(sizeof(DirectX::XMFLOAT4X4) * 2);

	}
	void OnImGuiRender()
	{
		

	}
	
	~ExampleLayer() {
	}
private:
	Scene scene;
	EditorCamera cam;
	Material* mat;
	TransformComponent* cmp;
};
class Sandbox : public Pistachio::Application
{
public:
	Sandbox() : Application("SandBoc", {true,0,0,0,0,0,0,0,0}) { PushLayer(new ExampleLayer("lol"));  }
	~Sandbox() { }
private:
};

Pistachio::Application* Pistachio::CreateApplication()
{
	return new Sandbox;
}
//int APIENTRY wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow)
//{
//	auto app = Pistachio::CreateApplication();
//	if (HMODULE mod = GetModuleHandleA("renderdoc.dll"))
//	{
//		pRENDERDOC_GetAPI RENDERDOC_GetAPI =
//			(pRENDERDOC_GetAPI)GetProcAddress(mod, "RENDERDOC_GetAPI");
//		int ret = RENDERDOC_GetAPI(eRENDERDOC_API_Version_1_1_2, (void**)&rdoc_api);
//		assert(ret == 1);
//	}
//	while(true) app->Step();
//	delete app;
//}

int main(int argc, char** argv)
{
	RENDERDOC_API_1_1_2* rdoc_api = NULL;
	auto app = Pistachio::CreateApplication();
	if (HMODULE mod = GetModuleHandleA("renderdoc.dll"))
	{
		pRENDERDOC_GetAPI RENDERDOC_GetAPI =
			(pRENDERDOC_GetAPI)GetProcAddress(mod, "RENDERDOC_GetAPI");
		int ret = RENDERDOC_GetAPI(eRENDERDOC_API_Version_1_1_2, (void**)&rdoc_api);
		assert(ret == 1);
	}
	int frame = 0;
	while (true)
	{
		frame++;
		if(frame == 1 && rdoc_api) rdoc_api->StartFrameCapture(NULL, NULL);
		app->Step();
		if (frame == 1 && rdoc_api) rdoc_api->EndFrameCapture(NULL, NULL);
	}
	delete app;
}
