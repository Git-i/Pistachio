#include "pch.h"
#include "Application.h"
namespace PistachioCS
{
	Application::Application(System::String^ name, ApplicationCreateInfo^ info)
	{
		Pistachio::ApplicationOptions opt;
		opt.exportTextures = info->exportAllocations;
		opt.headless = info->headless;
		for (uint32_t i = 0; i < 8; i++)
		{
			opt.gpu_luid.data[i] = info->luid ? info->luid[i] : 0;
		}
		m_ptr = new Pistachio::Application((char*)System::Runtime::InteropServices::Marshal::StringToHGlobalAnsi(name).ToPointer(), opt);
	}
	API Application::GetAPI()
	{
		RHI::API api = Pistachio::RendererBase::GetAPI();
		switch (api)
		{
		case RHI::API::DX12: return API::DirectX12;
			break;
		case RHI::API::Vulkan: return API::Vulkan;
			break;
		default: return API::Invalid;
			break;
		}
	}
	void Application::PushLayer(Layer^ layer)
	{
		m_ptr->PushLayer(layer->m_ptr);
	}
	void Application::Run()
	{
		m_ptr->Run();
	}
}
