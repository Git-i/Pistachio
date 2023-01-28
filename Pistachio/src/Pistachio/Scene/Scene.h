#pragma once
#include "entt.hpp"
#include "Pistachio/Renderer/RenderTexture.h"
#include "Pistachio/Renderer/EditorCamera.h"
namespace Pistachio {
	class Entity;
	class Scene {
	public:
		Scene();
		~Scene();
		Entity CreateEntity(const std::string& name = "");
		void OnUpdateEditor(float delta, EditorCamera& camera);
		void OnUpdateRuntime(float delta);
		void OnViewportResize(unsigned int width, unsigned int height);
		void DestroyEntity(Entity entity);
		Entity GetPrimaryCameraEntity();
	private:
		template<typename T>
		void OnComponentAdded(Entity entity, T& component);
	private:
		entt::registry m_Registry;
		friend class Entity;
		friend class SceneHierarchyPanel;
		friend class SceneSerializer;
		unsigned int m_viewportWidth, m_ViewportHeight;
	};
}