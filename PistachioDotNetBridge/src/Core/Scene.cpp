#include "pch.h"
#include "Scene.h"
const DirectX::SimpleMath::Matrix DirectX::SimpleMath::Matrix::Identity = DirectX::SimpleMath::Matrix(1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1);
namespace PistachioCS
{
	Scene::Scene(float width, float height)
	{
		Pistachio::SceneDesc dsc;
		dsc.Resolution = { width, height };
		m_ptr = new Pistachio::Scene(dsc);
		
	}
	Entity^ Scene::CreateEntity(System::String^ name)
	{
		Pistachio::Entity e = m_ptr->CreateEntity((char*)System::Runtime::InteropServices::Marshal::StringToHGlobalAnsi(name).ToPointer());
		return gcnew Entity(e, m_ptr);
	}
	void Scene::OnUpdateEditor(float delta, EditorCamera^ camera)
	{
		return m_ptr->OnUpdateEditor(delta, *camera->m_ptr);
	}
	System::IntPtr Scene::GetImage()
	{
		unsigned char* a = new unsigned char[1280 * 720 * 4];
		m_ptr->GetRenderedScene().GetRenderTexture().CopyToCPUBuffer(a);
		return System::IntPtr(a);
	}
	void Scene::FreeImage(System::IntPtr^ ptr)
	{
		delete ptr->ToPointer();
	}
}