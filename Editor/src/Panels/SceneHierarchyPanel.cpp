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
		Entity root = { (entt::entity)0, m_Context.get() };
		ImGuiTreeNodeFlags flags = ((m_SelectionContext == root) ? ImGuiTreeNodeFlags_Selected : 0) |ImGuiTreeNodeFlags_DefaultOpen| ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_SpanAvailWidth;
		bool opened = ImGui::TreeNodeEx((void*)(uint64_t)(unsigned int)root, flags, "Scene Root");
		if (ImGui::IsItemClicked())
		{
			m_SelectionContext = root;
		}
		if (ImGui::BeginDragDropTarget())
		{
			const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ENTITY");
			if (payload)
			{
				uint32_t* data = (uint32_t*)payload->Data;
				Entity other = { (entt::entity)*data, m_Context.get() };
				auto& pc = other.GetComponent<ParentComponent>();
				pc.parentID = (uint32_t)root;
			}
			ImGui::EndDragDropTarget();
		}
		if (opened)
		{
			m_Context->m_Registry.each([&](auto entityID) {
				Entity child{ entityID, m_Context.get() };
			if (child.GetComponent<ParentComponent>().parentID == (uint32_t)root)
				DrawEntityNode(child);
				});
			ImGui::TreePop();
		}
		if (ImGui::IsMouseDown(0) && ImGui::IsWindowHovered())
			m_SelectionContext = {};
		if (ImGui::BeginPopupContextWindow(0, 1 | ImGuiPopupFlags_NoOpenOverItems))
		{
			if (ImGui::MenuItem("Create New Entity"))
			{
				m_SelectionContext = m_Context->CreateEntity();
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
		if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceNoPreviewTooltip))
		{
			uint32_t data = (uint32_t)entity;
			ImGui::SetDragDropPayload("ENTITY", &data, sizeof(uint32_t), ImGuiCond_Once);
			ImGui::EndDragDropSource();
		}
		if (ImGui::BeginDragDropTarget())
		{
			const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ENTITY");
			if (payload)
			{
				uint32_t* data = (uint32_t*)payload->Data;
				Entity other = { (entt::entity)*data, m_Context.get() };
				auto& pc = other.GetComponent<ParentComponent>();
				pc.parentID = (uint32_t)entity;
			}
			ImGui::EndDragDropTarget();
		}
		bool entityDeleted = false;
		if (ImGui::BeginPopupContextItem())
		{
			if (ImGui::MenuItem("Delete Entity")) {
				entityDeleted = true;
			}
			ImGui::EndPopup();
		}
		if (ImGui::BeginPopupContextItem())
		{
			if (ImGui::MenuItem("Duplicate Entity")) {
				m_SelectionContext = m_Context->DuplicateEntity(entity);
			}
			ImGui::EndPopup();
		}
		if (opened)
		{
			m_Context->m_Registry.each([&](auto entityID) {
			Entity child{ entityID, m_Context.get() };
			if (child.GetComponent<ParentComponent>().parentID == (uint32_t)entity)
				DrawEntityNode(child);
			});
			ImGui::TreePop();
		}
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
	static bool DrawVec3Control(const std::string& label, float* value, float reset = 0.f)
	{
		bool modified = false;
		static int formatindex[3] = { 0.0,0.0,0.0 };
		const char* formats[] = { "%.3f", "%.2f", "%.1f", "%.0f" };
		ImGui::Columns(2, "transformColunm");
		ImGui::PushID(label.c_str());
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
		{
			value[0] = reset;
			modified = true;
		}
		ImGui::PopStyleColor();
		ImGui::SameLine();
		if(ImGui::DragFloat("##x", value, 0.1f, 0.f, 0.f, formats[GetFormatIndex(value[0], ImGui::GetCurrentWindow()->GetID("##x"))], ImGuiSliderFlags_NoRoundToFormat)) modified = true;
		ImGui::PopItemWidth();
		ImGui::SameLine();

		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4{ 0.2f, 0.7f, 0.3f, 1.0f });
		if (ImGui::Button("Y", buttonSize))
		{
			value[1] = reset;
			modified = true;
		}
		ImGui::PopStyleColor();
		ImGui::SameLine();
		if(ImGui::DragFloat("##y", &value[1], 0.1f, 0.f, 0.f, formats[GetFormatIndex(value[1], ImGui::GetCurrentWindow()->GetID("##y"))], ImGuiSliderFlags_NoRoundToFormat)) modified = true;
		ImGui::PopItemWidth();
		ImGui::SameLine();

		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4{ 0.1f, 0.25f, 0.95f, 1.0f });
		if (ImGui::Button("Z", buttonSize))
		{
			value[2] = reset;
			modified = true;
		}
		ImGui::PopStyleColor();
		ImGui::SameLine();
		if(ImGui::DragFloat("##z", &value[2], 0.1f, 0.f, 0.f, formats[GetFormatIndex(value[2], ImGui::GetCurrentWindow()->GetID("##z"))], ImGuiSliderFlags_NoRoundToFormat)) modified = true;
		ImGui::PopItemWidth();
		ImGui::PopStyleVar();
		ImGui::PopStyleColor(3);
		ImGui::Columns(1);
		ImGui::PopID();
		return modified;
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
		if ((uint32_t)m_SelectionContext != 0)
		{
			if (ImGui::Button("Add Component"))
				ImGui::OpenPopup("AddComponent");
		}

		if (ImGui::BeginPopup("AddComponent"))
		{
			if (!m_SelectionContext.HasComponent<CameraComponent>())
			{
				if (ImGui::MenuItem("Camera"))
				{
					m_SelectionContext.AddComponent<CameraComponent>();
					ImGui::CloseCurrentPopup();
				}
			}
			if (!m_SelectionContext.HasComponent<SpriteRendererComponent>())
			{
				if (ImGui::MenuItem("Sprite Renderer"))
				{
					m_SelectionContext.AddComponent<SpriteRendererComponent>();
					ImGui::CloseCurrentPopup();
				}
			}
			if(ImGui::BeginMenu("Physics"))
			{
				if (!m_SelectionContext.HasComponent<RigidBodyComponent>())
				{
					if (ImGui::MenuItem("Rigid Body"))
					{
						m_SelectionContext.AddComponent<RigidBodyComponent>();
						ImGui::CloseCurrentPopup();
					}
				}
				if (!m_SelectionContext.HasComponent<BoxColliderComponent>())
				{
					if (ImGui::MenuItem("Box Collider"))
					{
						m_SelectionContext.AddComponent<BoxColliderComponent>();
						ImGui::CloseCurrentPopup();
					}
				}
				if (!m_SelectionContext.HasComponent<SphereColliderComponent>())
				{
					if (ImGui::MenuItem("Sphere Collider"))
					{
						m_SelectionContext.AddComponent<SphereColliderComponent>();
						ImGui::CloseCurrentPopup();
					}
				}
				if (!m_SelectionContext.HasComponent<CapsuleColliderComponent>())
				{
					if (ImGui::MenuItem("Capsule Collider"))
					{
						m_SelectionContext.AddComponent<CapsuleColliderComponent>();
						ImGui::CloseCurrentPopup();
					}
				}
				if (!m_SelectionContext.HasComponent<PlaneColliderComponent>())
				{
					if (ImGui::MenuItem("Plane Collider"))
					{
						m_SelectionContext.AddComponent<PlaneColliderComponent>();
						ImGui::CloseCurrentPopup();
					}
				}
				ImGui::EndMenu();
			}
			if(ImGui::BeginMenu("Renderer"))
			{
				if (!m_SelectionContext.HasComponent<MeshRendererComponent>())
				{
					if (ImGui::MenuItem("Mesh Renderer"))
					{
						m_SelectionContext.AddComponent<MeshRendererComponent>();
						ImGui::CloseCurrentPopup();
					}
				}
				if (!m_SelectionContext.HasComponent<LightComponent>())
				{
					if (ImGui::MenuItem("Light"))
					{
						m_SelectionContext.AddComponent<LightComponent>();
						ImGui::CloseCurrentPopup();
					}
				}
				ImGui::EndMenu();
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
				Vector3 rotationDeg = tc.RotationEulerHint;
				rotationDeg = { DirectX::XMConvertToDegrees(rotationDeg.x),DirectX::XMConvertToDegrees(rotationDeg.y),DirectX::XMConvertToDegrees(rotationDeg.z)};
				if(DrawVec3Control("Translation", (float*)&tc.Translation))
					m_SelectionContext.GetComponent<TransformComponent>().bDirty = true;
				if(DrawVec3Control("Rotation", (float*)&rotationDeg)) 
					m_SelectionContext.GetComponent<TransformComponent>().bDirty = true;
				if(DrawVec3Control("Scale", (float*)&tc.Scale, 1.f))
					m_SelectionContext.GetComponent<TransformComponent>().bDirty = true;
				tc.RotationEulerHint = { DirectX::XMConvertToRadians(rotationDeg.x), DirectX::XMConvertToRadians(rotationDeg.y), DirectX::XMConvertToRadians(rotationDeg.z), 1.f };
				tc.RecalculateRotation();
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
		DrawComponent<MeshRendererComponent>("Mesh Renderer", entity, [](auto& component) {
			auto mat = GetAssetManager()->GetMaterialResource(component.material);
			ImGui::Button("Mesh");
			if (ImGui::BeginDragDropTarget())
			{
				const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("3D_MODEL");
				if (payload)
				{
					const wchar_t* data = (const wchar_t*)payload->Data;
					auto path = std::filesystem::path("assets") / data;
					if (component.Mesh) {
						component.Mesh->CreateStack(path.string().c_str());
					}
					else {
						component.Mesh = std::shared_ptr<Mesh>(Pistachio::Mesh::Create(path.string().c_str()));
					}
				}
				ImGui::EndDragDropTarget();
			}
			ImGui::Button("Diffuse");
			if (ImGui::BeginDragDropTarget())
			{
				const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("TEXTURE");
				if (payload)
				{
					const wchar_t* data = (const wchar_t*)payload->Data;
					auto path = std::filesystem::path("assets") / data;
					mat->diffuseTex = GetAssetManager()->CreateTexture2DAsset(path.string().c_str());
				}
				ImGui::EndDragDropTarget();
			}
			ImGui::Button("Metallic");
			if (ImGui::BeginDragDropTarget())
			{
				const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("TEXTURE");
				if (payload)
				{
					const wchar_t* data = (const wchar_t*)payload->Data;
					auto path = std::filesystem::path("assets") / data;
					mat->metallicTex = GetAssetManager()->CreateTexture2DAsset(path.string().c_str());
				}
				ImGui::EndDragDropTarget();
			}
			ImGui::Button("Roughness");
			if (ImGui::BeginDragDropTarget())
			{
				const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("TEXTURE");
				if (payload)
				{
					const wchar_t* data = (const wchar_t*)payload->Data;
					auto path = std::filesystem::path("assets") / data;
					mat->roughnessTex = GetAssetManager()->CreateTexture2DAsset(path.string().c_str());
				}
				ImGui::EndDragDropTarget();
			}
			ImGui::Button("Normal");
			if (ImGui::BeginDragDropTarget())
			{
				const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("TEXTURE");
				if (payload)
				{
					const wchar_t* data = (const wchar_t*)payload->Data;
					auto path = std::filesystem::path("assets") / data;
					mat->normalTex = GetAssetManager()->CreateTexture2DAsset(path.string().c_str());
				}
				ImGui::EndDragDropTarget();
			}
			ImGui::ColorEdit3("Color", (float*)&mat->diffuseColor);
			ImGui::SliderFloat("Metallic Fac", &mat->metallic, 0.0, 1.0);
			ImGui::SliderFloat("Rougness Fac", &mat->roughness, 0.0, 1.0);
		});
		DrawComponent<LightComponent>("Light", entity, [](auto& component) {
			const char* lightTypeStrings[] = { "Directional Light", "Point Light", "Spot Light"};
			const char* currentLightTypeString = lightTypeStrings[component.Type];
			if (ImGui::BeginCombo("Light Type", currentLightTypeString))
			{
				for (int i = 0; i < 3; i++)
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
			ImGui::DragFloat("Intensity", &component.Intensity, 1.f, 0.f, 100.f, "%.3f", ImGuiSliderFlags_AlwaysClamp);
			if (component.CastShadow)
			{
				if (ImGui::BeginCombo("Shadow Map Size", std::to_string(component.shadowMap.GetSize()).c_str()))
				{
					for (int i = 0; i < 4; i++)
					{
						bool isSelected = component.shadowMap.GetSize() == 1024 * std::pow(2, i);
						if (ImGui::Selectable(std::to_string((int)(1024 * std::pow(2, i))).c_str(), isSelected))
						{
							component.shadowMap.UpdateSize(1024 * std::pow(2, i));
						}
					}
					ImGui::EndCombo();
				}
			}
			if (ImGui::Checkbox("Cast Shadow", &component.CastShadow))
			{ 
				if (!component.CastShadow)
				{
					component.shadowMap.~ShadowMap();
				}
				else
				{
					component.shadowMap.Create(1024);
				}
			}
			if (component.Type == 1)
			{
				ImGui::DragFloat("Max. Distance", &component.exData.z, 1, 0, FLT_MAX, "%.3f", ImGuiSliderFlags_AlwaysClamp);
			}
			if (component.Type == 2)
			{
				float outercone = DirectX::XMScalarACos(component.exData.x);
				float innercone = DirectX::XMScalarACos(component.exData.y);
				ImGui::SliderAngle("Outer Cone", (float*)&outercone, DirectX::XMConvertToDegrees(innercone), 90.f);
				ImGui::SliderAngle("Inner Cone", (float*)&innercone, 0, DirectX::XMConvertToDegrees(outercone));
				ImGui::DragFloat("Max. Distance", &component.exData.z, 1, 0, FLT_MAX, "%.3f", ImGuiSliderFlags_AlwaysClamp);
				component.exData.x = DirectX::XMScalarCos(outercone);
				component.exData.y = DirectX::XMScalarCos(innercone);
			}
		});
		DrawComponent<RigidBodyComponent>("Rigid Body", entity, [](auto& component) {
			const char* BodyTypeStrings[] = { "Static", "Dynamic","!Kinematic"};
			const char* currentBodyTypeString = BodyTypeStrings[(int)component.type];
			if (ImGui::BeginCombo("Body Type", currentBodyTypeString))
			{
				for (int i = 0; i < 3; i++)
				{
					bool isSelected = currentBodyTypeString == BodyTypeStrings[i];
					if (ImGui::Selectable(BodyTypeStrings[i], isSelected))
					{
						currentBodyTypeString = BodyTypeStrings[i];
						component.type = (RigidBodyComponent::BodyType)i;
						if (i == 2)
							ImGui::SetTooltip("NOT SUPPORTED");
					}
				}
				ImGui::EndCombo();
			}
			ImGui::DragFloat("Density", &component.Density);
			ImGui::DragFloat("Static Friction", &component.StaticFriction);
			ImGui::DragFloat("Dynamic Friction", &component.DynamicFriction);
			ImGui::DragFloat("Restitution", &component.Restitution);
		});
		DrawComponent<BoxColliderComponent>("Box Collider", entity, [](auto& component) {
			ImGui::DragFloat3("Size", (float*)& component.size);
			ImGui::DragFloat3("Offset", (float*)& component.offset);
		});
		DrawComponent<SphereColliderComponent>("Sphere Collider", entity, [](auto& component) {
			ImGui::DragFloat("Size", &component.size);
			ImGui::DragFloat3("Offset", (float*)& component.offset);
		});
		DrawComponent<CapsuleColliderComponent>("Capsule Collider", entity, [](auto& component) {
			ImGui::DragFloat("Size", &component.radius);
			ImGui::DragFloat("Size", &component.height);
			ImGui::DragFloat3("Offset", (float*)& component.offset);
		});
		DrawComponent<PlaneColliderComponent>("Plane Collider", entity, [](auto& component) {
			ImGui::DragFloat3("Offset", (float*)& component.offset);
		});
	}
}