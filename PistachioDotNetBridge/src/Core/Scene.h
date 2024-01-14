#pragma once
#define PT_CUSTOM_ENTRY
#include "Pistachio.h"
#include "ManagedBase.h"
#include "Entity.h"
#include "EditorCamera.h"

namespace PistachioCS
{
	public ref class Scene : ManagedBase<Pistachio::Scene>
	{
	public:
		Scene(float width, float height);
		Entity^ CreateEntity(System::String^ name);
		void OnUpdateEditor(float delta, EditorCamera^ camera);
	};
}
