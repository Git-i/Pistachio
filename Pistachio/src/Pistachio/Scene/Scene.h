#pragma once
#include "entt.hpp"
#include "Pistachio/Renderer/RenderTexture.h"
#include "Pistachio/Renderer/EditorCamera.h"
#include "Pistachio/Core/UUID.h"
#include "Pistachio\Renderer\Mesh.h"
#include "Pistachio\Allocators\AtlasAllocator.h"
namespace physx {
	class PxScene;
}
namespace Pistachio {
	class Entity;
	struct SceneDesc
	{
		DirectX::XMFLOAT2 Resolution;
		SceneDesc() : Resolution({1920, 1080}) {}
	};
	class Scene {
	public:
		Scene(SceneDesc desc  = SceneDesc());
		~Scene();
		Entity CreateEntity(const std::string& name = "");
		Entity DuplicateEntity(Entity e);
		Entity CreateEntityWithUUID(UUID ID, const std::string& name = "");
		void OnRuntimeStart();
		void OnRuntimeStop();
		void OnUpdateEditor(float delta, EditorCamera& camera);
		void OnUpdateRuntime(float delta);
		void OnViewportResize(unsigned int width, unsigned int height);
		void DestroyEntity(Entity entity);
		Entity GetRootEntity();
		Entity GetPrimaryCameraEntity();
		const RenderTexture& GetGBuffer() { return m_gBuffer; };
		const RenderTexture& GetRenderedScene() { return m_finalRender; };
	private:
		template<typename T> void OnComponentAdded(Entity entity, T& component);
		void UpdateObjectCBs();
		void SortMeshComponents();
		DirectX::XMMATRIX GetTransfrom(Entity e);
		AtlasAllocator sm_allocator;
	private:
		Pistachio::Mesh* ScreenSpaceQuad;
		entt::registry m_Registry;
		physx::PxScene* m_PhysicsScene = NULL;
		friend class Entity;
		friend class SceneHierarchyPanel;
		friend class SceneSerializer;
		unsigned int m_viewportWidth, m_ViewportHeight;
		RenderTexture m_gBuffer;
		RenderTexture m_finalRender;
	};
}