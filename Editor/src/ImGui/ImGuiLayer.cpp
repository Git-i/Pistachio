#include "ptpch.h"
#include "ImGuiLayer.h"
#include "Pistachio/Renderer/RendererBase.h"
#include "Pistachio/Core/Application.h"
#include "../../imguizmo/ImGuizmo.h"

namespace Pistachio {
	ImGuiLayer::ImGuiLayer()
		: Layer("ImGuiLayer")
	{
		ImGui_ImplWin32_EnableDpiAwareness();
		// Setup Dear ImGui context
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO(); (void)io;
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
		io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
		//io.ConfigFlags |= ImGuiConfigFlags_DpiEnableScaleFonts;
		io.ConfigFlags |= ImGuiConfigFlags_DpiEnableScaleViewports;
		ImGui::StyleColorsDark();
		io.Fonts->AddFontFromFileTTF("Cascadia.ttf", 14 * ((WindowData*)GetWindowDataPtr())->dpiscale);
		// When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones.
		ImGuiStyle& style = ImGui::GetStyle();
		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			style.WindowRounding = 0.0f;
		}
		SetDarkTheme();
		ImGui_ImplWin32_Init(Application::Get().GetWindow().pd.hwnd);
		ImGui_ImplDX11_Init(RendererBase::Getd3dDevice(), RendererBase::Getd3dDeviceContext());
	}
	ImGuiLayer::~ImGuiLayer()
	{
		
	}
		
	void ImGuiLayer::OnUpdate(float delta)
	{
	}
	void ImGuiLayer::OnEvent(Event& event)
	{
		EventDispatcher dispatcher(event);
		dispatcher.Dispatch<KeyPressedEvent>(BIND_EVENT_FN(ImGuiLayer::OnKeyPressed));
		if (event.GetEventType() == EventType::MouseButtonPressed)
			event.Handled = ImGui::GetIO().WantCaptureMouse && BlockEvents;
		if (event.GetEventType() == EventType::MouseScrolled)
			event.Handled = ImGui::GetIO().WantCaptureMouse && BlockEvents;
	}
	void ImGuiLayer::OnImGuiRender()
	{
		End();
	}
	bool ImGuiLayer::OnKeyPressed(KeyPressedEvent& e)
	{
		if (ImGui::GetIO().WantCaptureKeyboard && BlockEvents)
		{
			return true;
		}
		return false;
	}
	void ImGuiLayer::Begin()
	{
		ImGui_ImplDX11_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();
		ImGuizmo::BeginFrame();
	}
	void ImGuiLayer::End()
	{
		return;
		ImGui::Render();
		ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
		if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			ImGui::UpdatePlatformWindows();
			ImGui::RenderPlatformWindowsDefault();
		}
	}
	void ImGuiLayer::OnAttach()
	{
		
	}
	void ImGuiLayer::SetGreenTheme()
	{
		ImGuiStyle& style = ImGui::GetStyle();
		style.Colors[ImGuiCol_WindowBg].w = 1.0f;
		ImVec4* colors = style.Colors;
		colors[ImGuiCol_Text] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
		colors[ImGuiCol_TextDisabled] = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
		colors[ImGuiCol_WindowBg] = ImVec4(0.06f, 0.06f, 0.06f, 1.00f);
		colors[ImGuiCol_ChildBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
		colors[ImGuiCol_PopupBg] = ImVec4(0.08f, 0.08f, 0.08f, 0.94f);
		colors[ImGuiCol_Border] = ImVec4(0.43f, 0.43f, 0.50f, 0.50f);
		colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
		colors[ImGuiCol_FrameBg] = ImVec4(0.22f, 0.22f, 0.23f, 0.54f);
		colors[ImGuiCol_FrameBgHovered] = ImVec4(0.11f, 0.10f, 0.10f, 0.40f);
		colors[ImGuiCol_FrameBgActive] = ImVec4(0.29f, 1.00f, 0.82f, 0.67f);
		colors[ImGuiCol_TitleBg] = ImVec4(0.04f, 0.04f, 0.04f, 1.00f);
		colors[ImGuiCol_TitleBgActive] = ImVec4(0.23f, 0.23f, 0.23f, 1.00f);
		colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.00f, 0.00f, 0.00f, 0.51f);
		colors[ImGuiCol_MenuBarBg] = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
		colors[ImGuiCol_ScrollbarBg] = ImVec4(0.02f, 0.02f, 0.02f, 0.53f);
		colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.31f, 0.31f, 0.31f, 1.00f);
		colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.41f, 0.41f, 0.41f, 1.00f);
		colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.51f, 0.51f, 0.51f, 1.00f);
		colors[ImGuiCol_CheckMark] = ImVec4(0.29f, 1.00f, 0.82f, 1.00f);
		colors[ImGuiCol_SliderGrab] = ImVec4(0.29f, 1.00f, 0.82f, 0.67f);
		colors[ImGuiCol_SliderGrabActive] = ImVec4(0.29f, 1.00f, 0.82f, 1.00f);
		colors[ImGuiCol_Button] = ImVec4(0.29f, 1.00f, 0.82f, 0.40f);
		colors[ImGuiCol_ButtonHovered] = ImVec4(0.29f, 1.00f, 0.82f, 1.00f);
		colors[ImGuiCol_ButtonActive] = ImVec4(0.29f, 1.00f, 0.82f, 1.00f);
		colors[ImGuiCol_Header] = ImVec4(0.29f, 1.00f, 0.82f, 0.31f);
		colors[ImGuiCol_HeaderHovered] = ImVec4(0.29f, 1.00f, 0.82f, 0.80f);
		colors[ImGuiCol_HeaderActive] = ImVec4(0.29f, 1.00f, 0.82f, 1.00f);
		colors[ImGuiCol_Separator] = ImVec4(0.43f, 0.43f, 0.50f, 0.50f);
		colors[ImGuiCol_SeparatorHovered] = ImVec4(0.10f, 0.40f, 0.75f, 0.78f);
		colors[ImGuiCol_SeparatorActive] = ImVec4(0.10f, 0.40f, 0.75f, 1.00f);
		colors[ImGuiCol_ResizeGrip] = ImVec4(0.26f, 0.59f, 0.98f, 0.20f);
		colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.67f);
		colors[ImGuiCol_ResizeGripActive] = ImVec4(0.26f, 0.59f, 0.98f, 0.95f);
		colors[ImGuiCol_Tab] = ImVec4(0.95f, 0.12f, 0.12f, 0.86f);
		colors[ImGuiCol_TabHovered] = ImVec4(0.29f, 1.00f, 0.82f, 0.80f);
		colors[ImGuiCol_TabActive] = ImVec4(0.29f, 1.00f, 0.82f, 1.00f);
		colors[ImGuiCol_TabUnfocused] = ImVec4(0.15f, 0.07f, 0.07f, 0.97f);
		colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.29f, 1.00f, 0.82f, 1.00f);
		colors[ImGuiCol_DockingPreview] = ImVec4(0.20f, 0.02f, 0.02f, 0.70f);
		colors[ImGuiCol_DockingEmptyBg] = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
		colors[ImGuiCol_PlotLines] = ImVec4(0.61f, 0.61f, 0.61f, 1.00f);
		colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
		colors[ImGuiCol_PlotHistogram] = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
		colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
		colors[ImGuiCol_TableHeaderBg] = ImVec4(0.19f, 0.19f, 0.20f, 1.00f);
		colors[ImGuiCol_TableBorderStrong] = ImVec4(0.31f, 0.31f, 0.35f, 1.00f);
		colors[ImGuiCol_TableBorderLight] = ImVec4(0.23f, 0.23f, 0.25f, 1.00f);
		colors[ImGuiCol_TableRowBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
		colors[ImGuiCol_TableRowBgAlt] = ImVec4(1.00f, 1.00f, 1.00f, 0.06f);
		colors[ImGuiCol_TextSelectedBg] = ImVec4(0.26f, 0.59f, 0.98f, 0.35f);
		colors[ImGuiCol_DragDropTarget] = ImVec4(1.00f, 1.00f, 0.00f, 0.90f);
		colors[ImGuiCol_NavHighlight] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
		colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
		colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
		colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.35f);
	}
	void ImGuiLayer::SetDarkTheme()
	{
		ImGui::StyleColorsDark();
		ImGuiStyle& style = ImGui::GetStyle();
		style.Colors[ImGuiCol_WindowBg].w = 1.f;
		ImVec4* colors = style.Colors;
		colors[ImGuiCol_WindowBg] = ImVec4(0.2f, 0.2f, 0.2f, 1.f);
		colors[ImGuiCol_Header] =        ImVec4(0.2f, 0.205f, 0.21f, 1.f);
		colors[ImGuiCol_HeaderHovered] = ImVec4(0.3f, 0.305f, 0.31f, 1.f);
		colors[ImGuiCol_HeaderActive] =  ImVec4(0.15f, 0.1505f, 0.151f, 1.00f);

		colors[ImGuiCol_Button] = ImVec4(0.2f, 0.205f, 0.21f, 1.f);
		colors[ImGuiCol_ButtonHovered] = ImVec4(0.3f, 0.305f, 0.31f, 1.f);
		colors[ImGuiCol_ButtonActive] = ImVec4(0.15f, 0.1505f, 0.151f, 1.00f);

		colors[ImGuiCol_FrameBg] = ImVec4(0.14f, 0.145f, 0.15f, 1.f);
		colors[ImGuiCol_FrameBgHovered] = ImVec4(0.3f, 0.305f, 0.31f, 1.f);
		colors[ImGuiCol_FrameBgActive] = ImVec4(0.15f, 0.1505f, 0.151f, 1.00f);

		colors[ImGuiCol_Tab] = ImVec4(0.15f, 0.1505f, 0.151f, 1.00f);
		colors[ImGuiCol_TabHovered] = ImVec4(0.38f, 0.3805f, 0.381f, 1.f);
		colors[ImGuiCol_TabActive] = ImVec4(0.28f, 0.2805f, 0.281f, 1.00f);
		colors[ImGuiCol_TabUnfocused] = ImVec4(0.15f, 0.1505f, 0.151f, 1.f);
		colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.2f, 0.205f, 0.21f, 1.00f);

		colors[ImGuiCol_TitleBg] = ImVec4(0.15f, 0.1505f, 0.151f, 1.00f);
		colors[ImGuiCol_TitleBgActive] = ImVec4(0.15f, 0.1505f, 0.151f, 1.00f);
		colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.95f, 0.1505f, 0.951f, 1.f);

		colors[ImGuiCol_Header] = ImVec4(0.34f, 0.3359f, 0.34f, 1.0f);
		colors[ImGuiCol_HeaderActive] = ImVec4(0.34f, 0.3359f, 0.34f, 1.0f);
		colors[ImGuiCol_HeaderHovered] = ImVec4(0.3f, 0.3f, 0.3f, 1.0f);
	}
	void ImGuiLayer::SetLightTheme()
	{
	}
}