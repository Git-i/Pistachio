#include "pch.h"
#include "Application.h"
namespace PistachioCS
{
	Application::Application(System::String^ name, bool headless)
	{
		if (!headless)
		{
			m_ptr = new Pistachio::Application((char*)System::Runtime::InteropServices::Marshal::StringToHGlobalAnsi(name).ToPointer());
			return;
		}
		m_ptr = new Pistachio::Application((char*)System::Runtime::InteropServices::Marshal::StringToHGlobalAnsi(name).ToPointer(), Pistachio::InitModeHeadless());
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
