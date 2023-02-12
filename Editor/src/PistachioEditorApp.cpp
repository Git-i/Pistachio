#define PISTACHIO_RENDER_API_DX11
#define IMGUI
#include "Pistachio/Core/Error.h"
#include "Pistachio/Core/EntryPoint.h"
#include "EditorLayer.h"

namespace Pistachio {
	class PistachioEditor : public Application
	{
	public:
		PistachioEditor() : Application("Pistachio Editor") { PushLayer(new EditorLayer("EditorLayer")); GetWindow().SetVsync(1); }
	private:
	};
	Pistachio::Application* Pistachio::CreateApplication()
	{
		return new PistachioEditor;
	}
}
