#pragma once
#include "Pistachio/Scene/Scene.h"
#include "Pistachio/Scene/Entity.h"
namespace Pistachio {
	class SceneHierarchyPanel {
	public:
		SceneHierarchyPanel(const Ref<Scene>& scene);
		SceneHierarchyPanel() = default;
		void SetContext(const Ref<Scene>& scene);
		void OnImGuiRender();
		void DrawEntityNode(Entity entity);
		void DrawEntityComponents(Entity entity);
		Entity GetSelectedEntity() const { return m_SelectionContext; }
		void SetSelectedEntity(Entity entity) { m_SelectionContext = entity; }
	private:
		Ref<Scene> m_Context;
		Entity m_SelectionContext;
	};
}
