#include "ptpch.h"
#include "entt.hpp"
#include "Entity.h"
namespace Pistachio {
	Entity::Entity(entt::entity handle, Scene* scene)
		: m_Scene(scene),
		m_EntityHandle(handle),
		version(m_Scene->m_Registry.current(m_EntityHandle))
	{
		
	}
}