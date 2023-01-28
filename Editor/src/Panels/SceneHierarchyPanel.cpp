#include "ptpch.h"
#include "SceneHierarchyPanel.h"
#include "imgui.h"
#include "imgui_internal.h"
#include "Pistachio/Scene/Components.h"
#include "Pistachio/Core/Window.h"
static float dpiscale;
namespace Pistachio {
	SceneHierarchyPanel::SceneHierarchyPanel(const Ref<Scene>& scene)
	{
		dpiscale = ((WindowData*)GetWindowDataPtr())->dpiscale;
		SetContext(scene);
	}

	void SceneHierarchyPanel::SetContext(const Ref<Scene>& scene)
	{
		m_Context = scene;
		m_SelectionContext = {};
	}

	void SceneHierarchyPanel::OnImGuiRender()
	{
		ImGui::Begin("Scene Hierarchy");
		m_Context->m_Registry.each([&](auto entityID) {
			Entity entity{ entityID, m_Context.get() };
			DrawEntityNode(entity);
		});

		if (ImGui::IsMouseDown(0) && ImGui::IsWindowHovered())
			m_SelectionContext = {};
		if (ImGui::BeginPopupContextWindow(0, 1 | ImGuiPopupFlags_NoOpenOverItems))
		{
			if (ImGui::MenuItem("Create New Entity"))
			{
				m_Context->CreateEntity("Empty Entity");
			}
			ImGui::EndPopup();
		}
		ImGui::End();

		ImGui::Begin("Properties");
		if (m_SelectionContext)
		{
			DrawEntityComponents(m_SelectionContext);

			
		}
		ImGui::End();
	}
	void SceneHierarchyPanel::DrawEntityNode(Entity entity)
	{
		auto& tag = entity.GetComponent<TagComponent>().Tag;
		ImGuiTreeNodeFlags flags = ((m_SelectionContext == entity) ? ImGuiTreeNodeFlags_Selected : 0) | ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_SpanAvailWidth;
		bool opened = ImGui::TreeNodeEx((void*)(uint64_t)(unsigned int)entity, flags, tag.c_str());
		if (ImGui::IsItemClicked())
		{
			m_SelectionContext = entity;
		}
		bool entityDeleted = false;
		if (ImGui::BeginPopupContextItem())
		{
			if (ImGui::MenuItem("Delete Entity")) {
				entityDeleted = true;
			}
			ImGui::EndPopup();
		}
		if (opened)
			ImGui::TreePop();
		if (entityDeleted) {
			if (entity == m_SelectionContext)
				m_SelectionContext = {};
			m_Context->DestroyEntity(entity);
		}
	}
	int GetFormatIndex(float value, ImGuiID id)
	{
		bool edit_start = false;
		edit_start = ((ImGui::GetHoveredID() == id) && (ImGui::IsMouseDoubleClicked(0) || (ImGui::IsMouseClicked(0) && (ImGui::IsKeyDown(ImGuiKey_LeftCtrl) || ImGui::IsKeyDown(ImGuiKey_RightCtrl))))) ? true : edit_start;
		if (edit_start)
		{
			edit_start = ImGui::IsItemActive();
			return 0;
		}
		int index = (int)(std::clamp<float>(std::abs(value) * 0.15, 0.f, 3.f));
		if (DirectX::XMScalarNearEqual(ImGui::GetWindowWidth(), ImGui::GetStyle().WindowMinSize.x, 10.f))
			return index > 2 ? index : 2;
		return index;
	}
	static void DrawVec3Control(const std::string& label, float* value, float reset = 0.f)
	{
		static int formatindex[3] = { 0.0,0.0,0.0 };
		ImGui::PushID(label.c_str());
		const char* formats[] = { "%.3f", "%.2f", "%.1f", "%.0f" };
		ImGui::Columns(2);
		//ImGui::SetColumnWidth(0, 150);
		ImGui::Text(label.c_str());
		ImGui::NextColumn();
		ImGui::PushMultiItemsWidths(3, ImGui::CalcItemWidth() );
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.f, 0.f));
		float lineheight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.f;
		ImVec2 buttonSize = { lineheight, lineheight };

		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.3f, 0.3f, 0.3f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.6f, 0.6f, 0.6f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.3f, 0.3f, 0.3f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4{ 0.8f, 0.1f, 0.15f, 1.0f });
		if (ImGui::Button("X", buttonSize))
			value[0] = reset;
		ImGui::PopStyleColor();
		ImGui::SameLine();
		ImGui::DragFloat("##x", value, 0.1f, 0.f, 0.f, formats[GetFormatIndex(value[0], ImGui::GetCurrentWindow()->GetID("##x"))], ImGuiSliderFlags_NoRoundToFormat);
		ImGui::PopItemWidth();
		ImGui::SameLine();

		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4{ 0.2f, 0.7f, 0.3f, 1.0f });
		if (ImGui::Button("Y", buttonSize))
			value[1] = reset;
		ImGui::PopStyleColor();
		ImGui::SameLine();
		ImGui::DragFloat("##y", &value[1], 0.1f, 0.f, 0.f, formats[GetFormatIndex(value[1], ImGui::GetCurrentWindow()->GetID("##y"))], ImGuiSliderFlags_NoRoundToFormat);
		ImGui::PopItemWidth();
		ImGui::SameLine();

		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4{ 0.1f, 0.25f, 0.95f, 1.0f });
		if (ImGui::Button("Z", buttonSize))
			value[2] = reset;
		ImGui::PopStyleColor();
		ImGui::SameLine();
		ImGui::DragFloat("##z", &value[2], 0.1f, 0.f, 0.f, formats[GetFormatIndex(value[2], ImGui::GetCurrentWindow()->GetID("##z"))], ImGuiSliderFlags_NoRoundToFormat);
		ImGui::PopItemWidth();
		ImGui::PopStyleVar();
		ImGui::PopStyleColor(3);
		ImGui::Columns(1);
		ImGui::PopID();
	}
	template<typename T, typename UIFunction>
	static void DrawComponent(const std::string& name, Entity entity, UIFunction uiFunction)
	{
		ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_AllowItemOverlap | ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_FramePadding;
		if (entity.HasComponent<T>())
		{
			float availableWidth = ImGui::GetContentRegionAvail().x;
			auto& component = entity.GetComponent<T>();
			bool open = ImGui::TreeNodeEx(name.c_str(), flags);
			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2{4.f,4.f});
			float lineheight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.f;
			ImVec2 buttonSize = { lineheight  , lineheight  };
			ImGui::SameLine(availableWidth - lineheight*0.5);
			if (ImGui::Button("+", buttonSize))
			{
				ImGui::OpenPopup("ComponentSettings");
			}
			bool removeComponent = false;
			if (ImGui::BeginPopup("ComponentSettings")) {
				if (ImGui::MenuItem("Remove Component"))
					removeComponent = true;
				ImGui::EndPopup();
			}
			if (open) {
				uiFunction(component);
				ImGui::TreePop();
				ImGui::Separator();
			}
			if (removeComponent)
				entity.RemoveComponent<T>();
			ImGui::PopStyleVar();
		}
	}
	void SceneHierarchyPanel::DrawEntityComponents(Entity entity)
	{
		ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_AllowItemOverlap | ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_FramePadding;
		if (entity.HasComponent<TagComponent>())
		{
			auto& tag = entity.GetComponent<TagComponent>().Tag;
			char buffer[256];
			memset(buffer, 0, sizeof(buffer));
			strcpy_s(buffer, tag.c_str());
			if (ImGui::InputText("##Tag", buffer, sizeof(buffer))) {
				tag = std::string(buffer);
			}
		}
		ImGui::SameLine();
		ImGui::PushItemWidth(-1);
		if (ImGui::Button("Add Component"))
			ImGui::OpenPopup("AddComponent");

		if (ImGui::BeginPopup("AddComponent"))
		{
			if (ImGui::MenuItem("Camera"))
			{
				if (!m_SelectionContext.HasComponent<CameraComponent>())
					m_SelectionContext.AddComponent<CameraComponent>();
				ImGui::CloseCurrentPopup();
			}
			if (ImGui::MenuItem("Sprite Renderer"))
			{
				if (!m_SelectionContext.HasComponent<SpriteRendererComponent>())
					m_SelectionContext.AddComponent<SpriteRendererComponent>();
				ImGui::CloseCurrentPopup();
			}
			if (ImGui::MenuItem("Light"))
			{
				if (!m_SelectionContext.HasComponent<LightComponent>())
					m_SelectionContext.AddComponent<LightComponent>();
				ImGui::CloseCurrentPopup();
			}
			ImGui::EndPopup();
		}
		ImGui::PopItemWidth();
		ImGui::Separator();
		if (entity.HasComponent<TransformComponent>())
		{
			bool open = ImGui::TreeNodeEx("Transform", flags);
			
			if (open) {
				auto& tc = entity.GetComponent<TransformComponent>();
				DirectX::XMFLOAT3 rotationDeg;
				DirectX::XMStoreFloat3(&rotationDeg, tc.Rotation);
				rotationDeg = { DirectX::XMConvertToDegrees(rotationDeg.x),DirectX::XMConvertToDegrees(rotationDeg.y),DirectX::XMConvertToDegrees(rotationDeg.z)};
				DrawVec3Control("Translation", (float*)&tc.Translation);
				DrawVec3Control("Rotation", (float*)&rotationDeg);
				DrawVec3Control("Scale", (float*)&tc.Scale, 1.f);
				tc.Rotation = { DirectX::XMConvertToRadians(rotationDeg.x),DirectX::XMConvertToRadians(rotationDeg.y),DirectX::XMConvertToRadians(rotationDeg.z),1.f };
				ImGui::TreePop();
			}
			ImGui::Separator();
		}
		DrawComponent<CameraComponent>("Camera Component", entity, [](auto& cameracomponent) {
			const char* projectionTypeStrings[] = { "Perspective", "Orthographic" };
			const char* currentProjectionTypeString = projectionTypeStrings[((int)(cameracomponent.camera.GetProjectionType()))];
			ImGui::Checkbox("Primary", &cameracomponent.Primary);
			if (ImGui::BeginCombo("Projection Type", currentProjectionTypeString))
			{
				for (int i = 0; i < 2; i++)
				{
					bool isSelected = currentProjectionTypeString == projectionTypeStrings[i];
					if (ImGui::Selectable(projectionTypeStrings[i], isSelected))
					{
						currentProjectionTypeString = projectionTypeStrings[i];
						cameracomponent.camera.SetProjectionType((SceneCamera::ProjectionType)i);
					}
				}
				ImGui::EndCombo();
			}
			if (cameracomponent.camera.GetProjectionType() == SceneCamera::ProjectionType::Perspective)
			{
				if (ImGui::SliderAngle("FOV", &cameracomponent.camera.m_perspsize)) cameracomponent.camera.SetProjectionType(SceneCamera::ProjectionType::Perspective);
				if (ImGui::DragFloat("Near Plane", &cameracomponent.camera.m_perspnear, 0.1f, 0.01f, 10000.f, "%.3f", ImGuiSliderFlags_AlwaysClamp)) cameracomponent.camera.SetProjectionType(SceneCamera::ProjectionType::Perspective);
				if (ImGui::DragFloat("Far Plane", &cameracomponent.camera.m_perspfar, 0.1f, cameracomponent.camera.m_perspnear + 0.5f, 10000.f, "%.3f", ImGuiSliderFlags_AlwaysClamp)) cameracomponent.camera.SetProjectionType(SceneCamera::ProjectionType::Perspective);
			}
			if (cameracomponent.camera.GetProjectionType() == SceneCamera::ProjectionType::Orthographic)
			{
				if (ImGui::DragFloat("Size", &cameracomponent.camera.m_orthosize, 0.1f, 0.01f, 10000.f, "%.3f", ImGuiSliderFlags_AlwaysClamp)) cameracomponent.camera.SetProjectionType(SceneCamera::ProjectionType::Orthographic);
				if (ImGui::DragFloat("Near Plane", &cameracomponent.camera.m_orthonear)) cameracomponent.camera.SetProjectionType(SceneCamera::ProjectionType::Orthographic);
				if (ImGui::DragFloat("Far Plane", &cameracomponent.camera.m_orthofar)) cameracomponent.camera.SetProjectionType(SceneCamera::ProjectionType::Orthographic);
			}
		});
		DrawComponent<SpriteRendererComponent>("Sprite Renderer", entity, [](auto& component) {
			ImGui::ColorEdit4("Color", (float*)&component.Color);
		});
		DrawComponent<LightComponent>("Light", entity, [](auto& component) {
			const char* lightTypeStrings[] = { "Directional Light", "Point Light" };
			const char* currentLightTypeString = lightTypeStrings[component.Type];
			if (ImGui::BeginCombo("Light Type", currentLightTypeString))
			{
				for (int i = 0; i < 2; i++)
				{
					bool isSelected = currentLightTypeString == lightTypeStrings[i];
					if (ImGui::Selectable(lightTypeStrings[i], isSelected))
					{
						currentLightTypeString = lightTypeStrings[i];
						component.Type = i;
					}
				}
				ImGui::EndCombo();
			}
			ImGui::ColorEdit3("Light Color", (float*)&component.color);
			ImGui::DragFloat("Intensity", &component.Intensity);
		});
	}
}