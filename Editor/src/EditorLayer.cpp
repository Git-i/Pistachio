#include "EditorLayer.h"
#include <DirectXColors.h>
#include "Pistachio/Scene/SceneSerializer.h"
#include "Pistachio/Utils/PlatformUtils.h"
#include "ImGuizmo.h"
namespace Pistachio {
	EditorLayer::EditorLayer(const char* name) : Layer(name), 
		m_playButton(Texture2D::Create("resources/textures/icons/play_icon.png")), 
		m_stopButton(Texture2D::Create("resources/textures/icons/stop_icon.png")),
		m_translateButton(Texture2D::Create("resources/textures/icons/translate_icon.png")),
		m_rotateButton(Texture2D::Create("resources/textures/icons/rotate_icon.png")),
		m_scaleButton(Texture2D::Create("resources/textures/icons/scale_icon.png")),
		m_EntityTexture(Texture2D::Create(1, 1, TextureFormat::INT, nullptr, (TextureFlags)((int)TextureFlags::ALLOW_CPU_ACCESS_READ | (int)TextureFlags::USAGE_STAGING))),
		envshader(L"resources/shaders/vertex/background_vs.cso", L"resources/shaders/pixel/background.cso")
	{
		cube.CreateStack("cube.obj");
		envshader.CreateLayout(Pistachio::Mesh::GetLayout(), Pistachio::Mesh::GetLayoutSize());
	}
	void EditorLayer::OnUpdate(float delta)
	{
		PT_PROFILE_FUNCTION()
		switch(m_SceneState)
		{
			case SceneState::Edit:
			{
				m_EditorCamera.OnUpdate(delta);
				m_ActiveScene->OnUpdateEditor(delta, m_EditorCamera);
				break;
			}
			case SceneState::Play:
			{
				m_ActiveScene->OnUpdateRuntime(delta);
			}
		}
		Pistachio::RendererBase::SetCullMode(Pistachio::CullMode::Front);
		DirectX::XMFLOAT3X3 view;
		DirectX::XMStoreFloat3x3(&view, DirectX::XMLoadFloat4x4(reinterpret_cast<const DirectX::XMFLOAT4X4*>(&m_EditorCamera.GetViewMatrix())));
		Renderer::BeginScene(&m_EditorCamera, DirectX::XMMatrixInverse(nullptr,DirectX::XMLoadFloat3x3(&view)));
		Pistachio::Renderer::Submit(&cube, &envshader, &Renderer::DefaultMaterial, -1);
		Pistachio::RendererBase::SetCullMode(Pistachio::CullMode::Back);
	}
	ImVec2 wndPos;
	void EditorLayer::OnAttach()
	{
		m_ActiveScene = std::make_shared<Scene>();
		m_EditorCamera = EditorCamera(30.f, 1.6, 0.1f, 1000.f);
		Entity e = m_ActiveScene->CreateEntity("sphere");
		auto& mr = e.AddComponent<MeshRendererComponent>("circle.obj");
		m_SceneHierarchyPanel.SetContext(m_ActiveScene);
	}
	void EditorLayer::OnImGuiRender()
	{
		ImGui::DockSpaceOverViewport();
		ImGui::GetStyle().WindowMinSize.x = 270.f * ((WindowData*)GetWindowDataPtr())->dpiscale;
		ImGui::BeginMainMenuBar();
		if (ImGui::BeginMenu("File"))
		{
			if (ImGui::MenuItem("New", "Ctrl + N"))
				NewScene();
			if (ImGui::MenuItem("Open...", "Ctrl+O"))
				OpenScene();
			if (ImGui::MenuItem("Save As...", "Ctrl+Shift+S"))
				SaveSceneAs();
			if (ImGui::MenuItem("Exit", "Alt+F4"))
				PostMessageA(Application::Get().GetWindow().pd.hwnd, WM_CLOSE, 0, 0);
			ImGui::EndMenu();
		}
		static bool gbuffervisualize = false;
		if (ImGui::BeginMenu("Windows"))
		{
			if (ImGui::BeginMenu("Debug"))
			{
				if (ImGui::MenuItem("Gbuffer Visualizer"))
					gbuffervisualize = true;
				if (ImGui::MenuItem("Content Browser"))
					m_ContentBrowserPanel.activated = true;
				ImGui::EndMenu();
			}
			ImGui::EndMenu();
		}
		ImGui::EndMainMenuBar();
		auto target = RendererBase::GetmainRenderTargetView();
		RendererBase::Getd3dDeviceContext()->OMSetRenderTargets(1, &target, RendererBase::GetDepthStencilView());
		m_SceneHierarchyPanel.OnImGuiRender();
		m_ContentBrowserPanel.OnImGuiRender();
		m_ConsolePanel.OnImGuiRender();
		m_MaterialEditorPanel.OnImGuiRender();
		if(gbuffervisualize)
		{
			ImGui::Begin("Debug-GBuffer-Visualizer", &gbuffervisualize);
			static int which = 0;
			ImGui::SliderInt("Gbuffer-Index", &which, 0, 3);
			if(ImGui::GetWindowWidth() < ImGui::GetWindowHeight())
			ImGui::Image(m_ActiveScene->GetGBuffer().GetSRV(which).ptr, { ImGui::GetWindowWidth(), ImGui::GetWindowWidth() * wndheight / wndwith });
			else
			ImGui::Image(m_ActiveScene->GetGBuffer().GetSRV(which).ptr, { ImGui::GetWindowHeight() * wndwith/wndheight, ImGui::GetWindowHeight() });
			ImGui::End();
		}
		if (ImGui::Begin("Scene"))
		{
			auto offset = ImGui::GetCursorPos();
			Application::Get().GetImGuiLayer()->BlockEvents = !ImGui::IsWindowHovered();
			ImVec2 viewportSize = ImGui::GetContentRegionAvail();
			if (viewportSize.x != wndwith || viewportSize.y != wndheight) {
				m_ActiveScene->OnViewportResize(viewportSize.x, viewportSize.y);
				m_EditorCamera.SetViewportSize(viewportSize.x, viewportSize.y);
			}
			wndwith = viewportSize.x;
			wndheight = viewportSize.y;
			ImGui::Image(m_ActiveScene->GetRenderedScene().GetSRV(0).ptr, viewportSize);
			if (ImGui::BeginDragDropTarget())
			{
				const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM");
				if (payload)
				{
					const wchar_t* data = (const wchar_t*)payload->Data;
					OpenScene(std::filesystem::path("assets") / data);
				}
				ImGui::EndDragDropTarget();
			}
			wndPos = { ImGui::GetWindowPos().x + offset.x, ImGui::GetWindowPos().y + offset.y };
			DirectX::XMFLOAT2 MouseScreenPos = { ImGui::GetMousePos().x - ImGui::GetWindowPos().x + offset.x, ImGui::GetMousePos().y - ImGui::GetWindowPos().y - offset.y };
			m_ViewportHovered = ImGui::IsWindowHovered();

			// Gizmos
			Entity selectedEntity = m_SceneHierarchyPanel.GetSelectedEntity();
			if (selectedEntity && m_GizmoType != -1 && m_SceneState == SceneState::Edit)
			{
				auto& tc = selectedEntity.GetComponent<TransformComponent>();
				ImGuizmo::SetDrawlist(ImGui::GetWindowDrawList());
				ImGuizmo::SetRect(ImGui::GetWindowPos().x, ImGui::GetWindowPos().y, wndwith, wndheight);
				float tr[3] = { tc.Translation.x, tc.Translation.y, tc.Translation.z };
				float rt[3] = { tc.Rotation.x, tc.RotationEulerHint.y, tc.RotationEulerHint.z };
				float sc[3] = { tc.Scale.x, tc.Scale.y, tc.Scale.z };

				Pistachio::Matrix4 view;
				Pistachio::Matrix4 projection;
				Pistachio::Matrix4 transform;
				ImGuizmo::RecomposeMatrixFromComponents(tr, rt, sc, &transform.m[0][0]);
				view = m_EditorCamera.GetViewMatrix();
				projection = m_EditorCamera.GetProjection();

				bool snap = Input::IsKeyPressed(PT_KEY_LCONTROL);
				float snapvalue = m_GizmoType == ImGuizmo::OPERATION::ROTATE ? 45.f : 0.5f;
				float snapvalues[3] = { snapvalue, snapvalue, snapvalue };
				if (m_GizmoType == ImGuizmo::OPERATION::SCALE)
					ImGuizmo::Manipulate((float*)&view, (float*)&projection, (ImGuizmo::OPERATION)m_GizmoType, ImGuizmo::LOCAL, (float*)&transform, nullptr, snap ? snapvalues : nullptr);
				else
					ImGuizmo::Manipulate((float*)&view, (float*)&projection, (ImGuizmo::OPERATION)m_GizmoType, ImGuizmo::WORLD, (float*)&transform, nullptr, snap ? snapvalues : nullptr);
				if (ImGuizmo::IsUsing())
				{
					ImGuizmo::DecomposeMatrixToComponents((float*)&transform, tr, rt, sc);
					transform.Decompose(tc.Scale, tc.Rotation, tc.Translation);
					Vector3 deltaRotation = Vector3(DirectX::XMConvertToRadians(rt[0]), DirectX::XMConvertToRadians(rt[1]), DirectX::XMConvertToRadians(rt[2])) - tc.RotationEulerHint;
					tc.RotationEulerHint = deltaRotation + tc.RotationEulerHint;
					tc.bDirty = true;
				}

			}
			bool focused = true;
			if (focused)
			{
				ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 10);
				ImGui::PushStyleVar(ImGuiStyleVar_WindowMinSize, { 10,10 });
				ImGui::Begin("##control", nullptr, ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
				ImGui::SetWindowSize({ 276,50 }, ImGuiCond_Once);
				ImGui::SetWindowPos({ wndPos.x + 20.f, wndPos.y + 20.f }, ImGuiCond_Always);
				ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, { 14, 4 });
				ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, { 5, 1 });
				ImGui::PushStyleColor(ImGuiCol_ButtonHovered, { 0,0,0,0 });
				ImGui::PushStyleColor(ImGuiCol_ButtonActive, { 0,0,0,0 });
				bool translate, rotate, scale;
				if (m_GizmoType == ImGuizmo::OPERATION::TRANSLATE)
					translate = ImGui::ImageButton(m_translateButton->GetID().ptr, ImVec2(32, 32), { 0,0 }, { 1,1 }, -1, { 0,0,0,0 }, { 0.33, 0.41, 0.772, 1.0 });
				else
					translate = ImGui::ImageButton(m_translateButton->GetID().ptr, ImVec2(32, 32), {0,0}, {1,1}, -1, {0,0,0,0}, {1,1,1, 1.0});
				if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal | ImGuiHoveredFlags_NoSharedDelay))
					ImGui::SetTooltip("Translate (W)");
				ImGui::SameLine();
				if (m_GizmoType == ImGuizmo::OPERATION::ROTATE)
					rotate = ImGui::ImageButton(m_rotateButton->GetID().ptr, ImVec2(32, 32), {0,0}, {1,1}, -1, {0,0,0,0}, {0.33, 0.41, 0.772, 1.0});
				else
					rotate = ImGui::ImageButton(m_rotateButton->GetID().ptr, ImVec2(32, 32), {0,0}, {1,1}, -1, {0,0,0,0}, {1,1,1, 1.0});
				if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal | ImGuiHoveredFlags_NoSharedDelay))
					ImGui::SetTooltip("Rotate (E)");
				ImGui::SameLine();
				if (m_GizmoType == ImGuizmo::OPERATION::SCALE)
					scale = ImGui::ImageButton(m_scaleButton->GetID().ptr, ImVec2(32, 32), {0,0}, {1,1}, -1, {0,0,0,0}, {0.33, 0.41, 0.772, 1.0});
				else
					scale = ImGui::ImageButton(m_scaleButton->GetID().ptr, ImVec2(32, 32), {0,0}, {1,1}, -1, {0,0,0,0}, {1,1,1, 1.0});
				if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal | ImGuiHoveredFlags_NoSharedDelay))
					ImGui::SetTooltip("Scale (R)");
				if (translate)
					m_GizmoType = ImGuizmo::OPERATION::TRANSLATE;
				else if (rotate)
					m_GizmoType = ImGuizmo::OPERATION::ROTATE;
				else if (scale)
					m_GizmoType = ImGuizmo::OPERATION::SCALE;
				ImGui::PopStyleVar(2);
				ImGui::PopStyleColor(2);
				ImGui::End();
				ImGui::PopStyleVar(2);
			}
		}
		ImGui::End();
		UI_ToolBar();
	}
	void EditorLayer::UI_ToolBar()
	{
		ImGui::Begin("##toolbar", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoTitleBar);
		float size = ImGui::GetContentRegionAvail().y;
		ImGui::SetCursorPosX((ImGui::GetWindowWidth() - size) / 2);
		if (m_SceneState == SceneState::Edit)
		{
			if (ImGui::ImageButton(m_playButton->GetID().ptr, ImVec2(size, size)))
			{
				OnScenePlay();
			}
		}
		else if (m_SceneState == SceneState::Play)
		{
			if (ImGui::ImageButton(m_stopButton->GetID().ptr, ImVec2(size, size)))
			{
				OnSceneStop();
			}
		}
		ImGui::End();

	}
	void EditorLayer::OnScenePlay()
	{
		m_SceneState = SceneState::Play;
		m_ActiveScene->OnRuntimeStart();
	}
	void EditorLayer::OnSceneStop()
	{
		m_SceneState = SceneState::Edit;
		m_ActiveScene->OnRuntimeStop();
	}
	void EditorLayer::OnEvent(Event& event)
	{
		EventDispatcher dispatcher(event);
		dispatcher.Dispatch<KeyPressedEvent>(BIND_EVENT_FN(EditorLayer::OnKeyPressed));
		dispatcher.Dispatch<FileDropEvent>(BIND_EVENT_FN(EditorLayer::OnFileDrop));
		dispatcher.Dispatch<MouseButtonPressedEvent>(BIND_EVENT_FN(EditorLayer::OnMouseButtonPressed));
		m_EditorCamera.OnEvent(event);
	}
	bool EditorLayer::OnFileDrop(FileDropEvent& e)
	{
		OpenScene(e.GetFileName());
		
		return true;
	}
	bool EditorLayer::OnMouseButtonPressed(MouseButtonPressedEvent& e)
	{
		if (e.GetMouseButton() == 0 && !ImGuizmo::IsOver() && !Input::IsKeyPressed(PT_KEY_SHIFT))
			m_SceneHierarchyPanel.SetSelectedEntity(m_HoveredEntity);
		return false;
	}
	bool EditorLayer::OnKeyPressed(KeyPressedEvent& e)
	{
		if (e.isRepeat())
			return false;
		bool control = Input::IsKeyPressed(PT_KEY_CONTROL);
		bool shift = Input::IsKeyPressed(PT_KEY_SHIFT);
		Entity selected = m_SceneHierarchyPanel.GetSelectedEntity();
		switch (e.GetKeyCode())
		{
		case PT_KEY_S:
		{
			if (control && shift)
				SaveSceneAs();
			break;
		}
		case PT_KEY_N:
		{
			if (control)
				NewScene();
			break;
		}
		case PT_KEY_O:
		{
			if (control)
				OpenScene();
			break;
		}
		case PT_KEY_W:
		{
			m_GizmoType = ImGuizmo::OPERATION::TRANSLATE;
			break;
		}
		case PT_KEY_E:
		{
			m_GizmoType = ImGuizmo::OPERATION::ROTATE;
			break;
		}
		case PT_KEY_R:
		{
			m_GizmoType = ImGuizmo::OPERATION::SCALE;
			break;
		}
		case PT_KEY_Q:
		{
			m_GizmoType = -1;
			break;
		}
		case PT_KEY_D:
		{
			m_SceneHierarchyPanel.SetSelectedEntity(m_ActiveScene->DuplicateEntity(selected));
			break;
		}
		default:
			break;
		}
		return false;
	}
	void EditorLayer::NewScene()
	{
		m_ActiveScene = std::make_shared<Scene>();
		m_ActiveScene->OnViewportResize(wndwith, wndheight);
		m_SceneHierarchyPanel.SetContext(m_ActiveScene);
	}
	void EditorLayer::OpenScene()
	{
		std::string filepath = FileDialogs::OpenFile("Pistachio Scenes (*.ptscene)\0*.ptscene\0");
		if (!filepath.empty())
		{
			m_ActiveScene = std::make_shared<Scene>();
			m_ActiveScene->OnViewportResize(wndwith, wndheight);
			m_SceneHierarchyPanel.SetContext(m_ActiveScene);

			SceneSerializer serializer(m_ActiveScene);
			serializer.DeSerialize(filepath);
		}
	}
	void EditorLayer::OpenScene(const std::filesystem::path& filepath)
	{
		if (!filepath.empty())
		{
			m_ActiveScene = std::make_shared<Scene>();
			m_ActiveScene->OnViewportResize(wndwith, wndheight);
			m_SceneHierarchyPanel.SetContext(m_ActiveScene);

			SceneSerializer serializer(m_ActiveScene);
			serializer.DeSerialize(filepath.string());
		}
	}
	void EditorLayer::SaveSceneAs()
	{
		std::string filepath = FileDialogs::SaveFile("Pistachio Scenes (*.ptscene)\0*.ptscene\0");
		if (!filepath.empty())
		{
			SceneSerializer serializer(m_ActiveScene);
			serializer.Serialize(filepath);
		}
	}
	EditorLayer::~EditorLayer() {
		
	}
}