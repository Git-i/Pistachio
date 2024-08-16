#pragma once

#include "Pistachio/Core.h"
#include "Pistachio/Core/LayerStack.h"
#include "Pistachio/Event/Event.h"
#include "Pistachio/Core/Window.h"
#include "Pistachio/Event/KeyEvent.h"

#include "Pistachio/Renderer/Renderer.h"
#include "Pistachio/Renderer/Buffer.h"
#include "Pistachio/Renderer/Shader.h"
#include "Pistachio/Renderer/Camera.h"
#include "Pistachio/Core/Input.h"
#include <complex.h>
#include <memory>
namespace Pistachio {

	/*
	* Describes configuration for Pistachio Application
	* - Headless: Create Window for application (if yes, scenes would still render to textures)
	* - GPU LUID: if not zero, the specific gpu to use on creation
	*/
	struct PISTACHIO_API ApplicationOptions
	{
		bool headless = false;
		RHI::LUID gpu_luid{};
		bool exportTextures = false;
		bool forceSingleQueue;
		//using custom devices
		Internal_ID custom_device = nullptr;
		Internal_ID custom_instance = nullptr;
		Internal_ID custom_physical_device = nullptr;
		Internal_ID custom_direct_queue = nullptr;//required
		Internal_ID custom_compute_queue = nullptr;//optional
		RHI::QueueFamilyIndices indices;
		//logger options
		//leave null to log to stdout
		const char* log_file_name = nullptr;
		std::function<RHI::PhysicalDevice*(std::span<RHI::PhysicalDevice*>)> select_physical_device;
	};
	class PISTACHIO_API Application
	{
	public:
		Application(const char* name, const ApplicationOptions& options = ApplicationOptions());
		virtual ~Application();
		void Run();
		void Step();
		void OnEvent(Event& e);
		void PushLayer(Layer* layer);
		void PushOverlay(Layer* overlay);
		bool OnWindowResize(WindowResizeEvent& e);
		void SetInputHandler(std::unique_ptr<InputHandler> handler);
		inline InputHandler& GetInputHandler() {return *handler;}
		static Application& Get();
		static bool Exists();
		inline Window& GetWindow() { return *m_Window; }
		void SetImGuiContext(void* ctx);
		void Stop() {m_Running = false;}
		bool IsHeadless() {
			return m_headless;
		};
	private:
		std::unique_ptr<InputHandler> handler;
		bool m_headless = false;
		LayerStack m_layerstack;
		std::unique_ptr<Window> m_Window;
		bool m_Running = true;
		bool m_minimized = false;
		static Application* s_Instance;
		std::chrono::time_point<std::chrono::high_resolution_clock> InitTime;
		std::chrono::milliseconds lastFrameTime;
		//LARGE_INTEGER frequency;
		double period;
		//LARGE_INTEGER ticks;
	};
	
	Application* CreateApplication();
}
