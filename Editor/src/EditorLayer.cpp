#include "EditorLayer.h"
#include <DirectXColors.h>
#include "Pistachio/Scene/SceneSerializer.h"
#include "Pistachio/Utils/PlatformUtils.h"
#include "ImGuizmo.h"
namespace Pistachio {
	EditorLayer::EditorLayer(const char* name) : Layer(name)
	{
		RenderTextureDesc rtDesc;
		rtDesc.width = 3840;
		rtDesc.height = 2400;
		rtDesc.miplevels = 1;
		rtDesc.Attachments = { TextureFormat::RGBA8U, TextureFormat::INT,TextureFormat::D32F};
		rtx.CreateStack(rtDesc);
	}
	void EditorLayer::OnUpdate(float delta)
	{
		PT_PROFILE_FUNCTION()
		float color[4] = { 1-.3f,1 - .3f,1 - .3f,1.f };
		float color1[4] = { -1,-1,-1,-1 };
		rtx.Clear(color1, 1);
		rtx.Clear(color, 0);
		rtx.Bind(0, 2);
		m_EditorCamera.OnUpdate(delta);
		m_ActiveScene->OnUpdateEditor(delta, m_EditorCamera);
	}
	
	void EditorLayer::OnAttach()
	{
		m_ActiveScene = std::make_shared<Scene>();
		m_EditorCamera = EditorCamera(30.f, 1.6, 0.1f, 1000.f);
		m_ActiveScene->CreateEntity("Mesh").AddComponent<MeshRendererComponent>().Mesh.CreateStack("sphere.obj");
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
		ImGui::EndMainMenuBar();
		auto target = RendererBase::GetmainRenderTargetView();
		RendererBase::Getd3dDeviceContext()->OMSetRenderTargets(1, &target, RendererBase::GetDepthStencilView());
		m_SceneHierarchyPanel.OnImGuiRender();
		m_ContentBrowserPanel.OnImGuiRender();
		ImGui::Begin("Scene");
		auto offset = ImGui::GetCursorPos();
		Application::Get().GetImGuiLayer()->BlockEvents = !ImGui::IsWindowHovered();
		ImVec2 viewportSize = ImGui::GetContentRegionAvail();
		if (viewportSize.x != wndwith || viewportSize.y != wndheight) {
			m_ActiveScene->OnViewportResize(viewportSize.x, viewportSize.y);
			m_EditorCamera.SetViewportSize(viewportSize.x, viewportSize.y);
		}
		wndwith = viewportSize.x;
		wndheight = viewportSize.y;
		ImGui::Image(rtx.GetSRV(0), viewportSize);
		if (ImGui::BeginDragDropTarget())
		{
			const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM");
			if (payload)
			{
				const wchar_t* data = (const wchar_t*)payload->Data;
				OpenScene(std::filesystem::path("assets")/data);
			}
			ImGui::EndDragDropTarget();
		}
		DirectX::XMFLOAT2 MouseScreenPos = { ImGui::GetMousePos().x - ImGui::GetWindowPos().x + offset.x, ImGui::GetMousePos().y - ImGui::GetWindowPos().y - offset.y };
		m_ViewportHovered = ImGui::IsWindowHovered();
		if(m_ViewportHovered)
		if (MouseScreenPos.x >= 0 && MouseScreenPos.y >= 0 && MouseScreenPos.x <= wndwith-1 && MouseScreenPos.y <= wndheight-1)
		{
			ID3D11Texture2D* pSelectedEntityTexture;
			D3D11_TEXTURE2D_DESC txDesc = {};
			txDesc.Format = DXGI_FORMAT_R32_SINT;
			txDesc.Height = 1;
			txDesc.Width = 1;
			txDesc.ArraySize = 1;
			txDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
			txDesc.Usage = D3D11_USAGE_STAGING;
			txDesc.SampleDesc.Count = 1;
			txDesc.SampleDesc.Quality = 0;
			RendererBase::Getd3dDevice()->CreateTexture2D(&txDesc, NULL, &pSelectedEntityTexture);
			ID3D11Resource* pSrcResource;
			rtx.GetRenderTexture(&pSrcResource, 1);
			D3D11_BOX sourceRegion;
			sourceRegion.left = MouseScreenPos.x * rtx.GetWidth()/wndwith;
			sourceRegion.right = sourceRegion.left + 1;
			sourceRegion.top = MouseScreenPos.y * rtx.GetHeight()/wndheight;
			sourceRegion.bottom = sourceRegion.top + 1;
			sourceRegion.front = 0;
			sourceRegion.back = 1;
			RendererBase::Getd3dDeviceContext()->CopySubresourceRegion(pSelectedEntityTexture, 0, 0, 0, 0, pSrcResource, 0, &sourceRegion);
			D3D11_MAPPED_SUBRESOURCE mappedResource;
			RendererBase::Getd3dDeviceContext()->Map(pSelectedEntityTexture, 0, D3D11_MAP_READ, 0, &mappedResource);
			m_HoveredEntity =  *(int*)mappedResource.pData == -1 ? Entity() : Entity(*(entt::entity*)mappedResource.pData, m_ActiveScene.get());
			pSrcResource->Release();
			pSelectedEntityTexture->Release();
		}



		// Gizmos
		Entity selectedEntity = m_SceneHierarchyPanel.GetSelectedEntity();
		if (selectedEntity && m_GizmoType != -1)
		{
			auto& tc = selectedEntity.GetComponent<TransformComponent>();
			ImGuizmo::SetDrawlist(ImGui::GetWindowDrawList());
			ImGuizmo::SetRect(ImGui::GetWindowPos().x, ImGui::GetWindowPos().y, wndwith, wndheight);
			float tr[3];
			DirectX::XMVectorGetByIndexPtr(tr, tc.Translation, 0);
			DirectX::XMVectorGetByIndexPtr(&tr[1], tc.Translation, 1);
			DirectX::XMVectorGetByIndexPtr(&tr[2], tc.Translation, 2);
			float rt[3] = {
			DirectX::XMConvertToDegrees(DirectX::XMVectorGetX(tc.Rotation)),
			DirectX::XMConvertToDegrees(DirectX::XMVectorGetY(tc.Rotation)),
			DirectX::XMConvertToDegrees(DirectX::XMVectorGetZ(tc.Rotation)),
			};
			float sc[3];
			DirectX::XMVectorGetByIndexPtr(sc, tc.Scale, 0);
			DirectX::XMVectorGetByIndexPtr(&sc[1], tc.Scale, 1);
			DirectX::XMVectorGetByIndexPtr(&sc[2], tc.Scale, 2);

			DirectX::XMFLOAT4X4 view;
			DirectX::XMFLOAT4X4 projection;
			DirectX::XMFLOAT4X4 transform;
			ImGuizmo::RecomposeMatrixFromComponents(tr, rt, sc, (float*)&transform);
			DirectX::XMStoreFloat4x4(&view, m_EditorCamera.GetViewMatrix());
			DirectX::XMStoreFloat4x4(&projection, m_EditorCamera.GetProjection());

			bool snap = Input::IsKeyPressed(PT_KEY_LCONTROL);
			float snapvalue = m_GizmoType == ImGuizmo::OPERATION::ROTATE ? 45.f : 0.5f;
			float snapvalues[3] = { snapvalue, snapvalue, snapvalue };
			if(m_GizmoType == ImGuizmo::OPERATION::SCALE)
				ImGuizmo::Manipulate((float*)&view, (float*)&projection, (ImGuizmo::OPERATION)m_GizmoType, ImGuizmo::LOCAL, (float*)&transform, nullptr, snap ? snapvalues : nullptr);
			else
				ImGuizmo::Manipulate((float*)&view, (float*)&projection, (ImGuizmo::OPERATION)m_GizmoType, ImGuizmo::WORLD, (float*)&transform, nullptr, snap ? snapvalues : nullptr);
			if (ImGuizmo::IsUsing())
			{
				ImGuizmo::DecomposeMatrixToComponents((float*)&transform, tr, rt, sc);
				tc.Translation = DirectX::XMVectorSet(tr[0], tr[1], tr[2],1.f);
				DirectX::XMVECTOR deltaRotation = DirectX::XMVectorSubtract(DirectX::XMVectorSet(DirectX::XMConvertToRadians(rt[0]), DirectX::XMConvertToRadians(rt[1]), DirectX::XMConvertToRadians(rt[2]), 1.f), tc.Rotation);
				tc.Rotation = DirectX::XMVectorAdd(deltaRotation, tc.Rotation);
				tc.Scale = DirectX::XMVectorSet(sc[0], sc[1], sc[2],1.f);
			}
		}
		ImGui::ShowDemoWindow();
		ImGui::End();
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