#pragma once
#include "Scene.h"
#include "Pistachio/Core/UUID.h"
struct IDComponent
{
	Pistachio::UUID uuid;
	IDComponent() = default;
	IDComponent(const IDComponent&) = default;
	IDComponent(Pistachio::UUID uuid) : uuid(uuid)
	{}
};
namespace Pistachio {
	class Entity {
	public:
		Entity() = default;
		Entity(entt::entity handle, Scene* scene);
		Entity(const Entity& other) = default;

		template<typename T, typename... Args>
		T& AddComponent(Args&&... args)
		{
			T& component = m_Scene->m_Registry.emplace<T>(m_EntityHandle, std::forward<Args>(args)...);
			m_Scene->OnComponentAdded<T>(*this, component);
			return component;
		}

		template<typename T>
		T& GetComponent() const
		{
			return m_Scene->m_Registry.get<T>(m_EntityHandle);
		}

		template<typename T>
		bool HasComponent()
		{
			return m_Scene->m_Registry.all_of<T>(m_EntityHandle);
		}

		template<typename T>
		bool RemoveComponent()
		{
			return m_Scene->m_Registry.remove<T>(m_EntityHandle);
		}
		UUID GetUUID() { return GetComponent<IDComponent>().uuid; }
		operator bool() const { return m_EntityHandle != entt::null; }
		operator entt::entity() const { return m_EntityHandle; }
		operator unsigned int() const { return (unsigned int)m_EntityHandle; }
		bool operator==(const Entity& other) const { return m_EntityHandle == other.m_EntityHandle && m_Scene == other.m_Scene; }
	private:
		entt::entity m_EntityHandle = entt::null;
		Scene* m_Scene = nullptr;
		friend class TransformComponent;
	};
}

