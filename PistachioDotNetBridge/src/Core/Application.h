#pragma once
#define PT_CUSTOM_ENTRY
#include "Pistachio.h"
#include "ManagedBase.h"
#include "Layer.h"

namespace PistachioCS {
	public ref class Application : public ManagedBase<Pistachio::Application>
	{
	public:
		Application(System::String^ name, bool headless);
		void PushLayer(Layer^ layer);
		void Run();
	};
}
