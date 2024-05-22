#pragma once
#include "Scene.h"
#include "Pistachio/Core/UUID.h"
struct PISTACHIO_API IDComponent
{
	Pistachio::UUID uuid;
	IDComponent() = default;
	IDComponent(const IDComponent&) = default;
	IDComponent(Pistachio::UUID uuid) : uuid(uuid)
	{}
};
namespace Pistachio {
	class PISTACHIO_API Entity {
	public:
		Entity() = default;
		Entity(entt::entity handle, Scene* scene);
		Entity(const Entity& other) = default;
		bool IsValid()
		{
			return 
			m_Scene->m_Registry.valid(m_EntityHandle) &&
			m_Scene->m_Registry.current(m_EntityHandle) == version;
		}
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
		Scene* m_Scene = nullptr;
		entt::entity m_EntityHandle = entt::null;
		entt::entt_traits<entt::entity>::version_type version;
		friend struct TransformComponent;
	};
}

