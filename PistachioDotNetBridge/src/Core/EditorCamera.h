#pragma once
#define PT_CUSTOM_ENTRY
#include "Pistachio.h"
#include "ManagedBase.h"

namespace PistachioCS
{
	public ref class EditorCamera : ManagedBase<Pistachio::EditorCamera>
	{
	public:
		EditorCamera(float fov, float aspect, float nearclip, float farclip) { m_ptr = new Pistachio::EditorCamera(fov, aspect, nearclip,farclip); }
		EditorCamera() { m_ptr = new Pistachio::EditorCamera(); }
		void OnUpdate(float delta) { m_ptr->OnUpdate(delta); }
		void UpdateViewportSize(float width, float height) { m_ptr->SetViewportSize(width, height); }
	};
}
