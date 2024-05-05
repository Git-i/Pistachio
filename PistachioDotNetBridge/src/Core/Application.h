#pragma once
#define PT_CUSTOM_ENTRY
#include "Pistachio.h"
#include "ManagedBase.h"
#include "Layer.h"

namespace PistachioCS {
	public value struct ApplicationCreateInfo
	{
		bool headless;
		array<byte>^ luid;
		bool exportAllocations;
		
	};
	public enum class API
	{
		Vulkan, DirectX12, Invalid
	};
	public ref class Application : public ManagedBase<Pistachio::Application>
	{
	public:
		Application(System::String^ name, ApplicationCreateInfo^ info);
		API GetAPI();
		void PushLayer(Layer^ layer);
		void Run();
	};
}
