#pragma once
#include "Pistachio.h"
#include "Panels/SceneHierarchyPanel.h"
#include "Panels/ContentBrowserPanel.h"
#include "Panels/ConsolePanel.h"
#include "Panels/MaterialEditorPanel.h"
#include "Pistachio/Renderer/EditorCamera.h"
#include "ImGui\ImGuiLayer.h"
namespace Pistachio {
	class EditorLayer : public Pistachio::Layer
	{
	public:
		EditorLayer(const char* name);
		void OnUpdate(float delta) override;
		void OnAttach() override;
		void OnImGuiRender();
		void OnEvent(Pistachio::Event& event) override;
		void UI_ToolBar();
		void OnScenePlay();
		void OnSceneStop();
		~EditorLayer();
	private:
		bool OnKeyPressed(Pistachio::KeyPressedEvent& e);
		bool OnMouseButtonPressed(MouseButtonPressedEvent& e);
		bool OnFileDrop(Pistachio::FileDropEvent& e);
		void NewScene();
		void OpenScene();
		void OpenScene(const std::filesystem::path& File);
		void SaveSceneAs();
	private:
		float wndwith = 0;
		float wndheight = 0;
		Ref<Scene> m_ActiveScene;
		Ref<Texture2D> m_playButton, m_stopButton, m_translateButton, m_rotateButton, m_scaleButton, m_EntityTexture;
		Mesh cube;
		Shader envshader;
		Entity m_HoveredEntity;
		Entity m_meshEntity;
		int m_GizmoType = -1;
		ImGuiLayer* imguiLayer;
		EditorCamera m_EditorCamera;
		SceneHierarchyPanel m_SceneHierarchyPanel;
		ContentBrowserPanel m_ContentBrowserPanel;
		MaterialEditorPanel m_MaterialEditorPanel;
		ConsolePanel m_ConsolePanel;
		bool m_ViewportHovered;

		enum class SceneState {
			Edit, Play = 1
		};

		SceneState m_SceneState = SceneState::Edit;
	};
}
