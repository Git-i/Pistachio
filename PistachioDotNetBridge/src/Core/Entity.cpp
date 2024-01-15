#include "pch.h"
#include "Entity.h"

namespace PistachioCS
{
	Entity::Entity(entt::entity handle, Pistachio::Scene* scene)
	{
		m_ptr = new Pistachio::Entity(handle, scene);
	}

	
	generic <typename T> where T : gcnew()
		T Entity::GetComponent()
	{
		if (T::typeid == IDComponent::typeid)
		{
			auto& comp = m_ptr->GetComponent<Pistachio::IDComponent>();
			auto comp_ptr = &comp;
			T retVal = gcnew T();
			((IDComponent^)retVal)->m_ptr = comp_ptr;
			return retVal;
		}
		else if (T::typeid == ParentComponent::typeid)
		{
			auto& comp = m_ptr->GetComponent<Pistachio::ParentComponent>();
			auto comp_ptr = &comp;
			T retVal = gcnew T();
			((ParentComponent^)retVal)->m_ptr = comp_ptr;
			return retVal;
		}
		else if (T::typeid == TagComponent::typeid)
		{
			auto& comp = m_ptr->GetComponent<Pistachio::TagComponent>();
			auto comp_ptr = &comp;
			T retVal = gcnew T();
			((TagComponent^)retVal)->m_ptr = comp_ptr;
			return retVal;
		}
		else if (T::typeid == TransformComponent::typeid)
		{
			auto& comp = m_ptr->GetComponent<Pistachio::TransformComponent>();
			auto comp_ptr = &comp;
			T retVal = gcnew T();
			((TransformComponent^)retVal)->m_ptr = comp_ptr;
			return retVal;
		}
		else if (T::typeid == MeshRendererComponent::typeid)
		{
			auto& comp = m_ptr->GetComponent<Pistachio::MeshRendererComponent>();
			auto comp_ptr = &comp;
			T retVal = gcnew T();
			((MeshRendererComponent^)retVal)->m_ptr = comp_ptr;
			return retVal;
		}
		throw gcnew System::ArgumentException(T::typeid->FullName, " is not a valid Pistachio component" );
		
	}
	generic <typename T> where T : gcnew()
		T Entity::AddComponent()
	{
		if (T::typeid == IDComponent::typeid)
		{
			auto& comp = m_ptr->AddComponent<Pistachio::IDComponent>();
			auto comp_ptr = &comp;
			T retVal = gcnew T();
			((IDComponent^)retVal)->m_ptr = comp_ptr;
			return retVal;
		}
		else if (T::typeid == ParentComponent::typeid)
		{
			auto& comp = m_ptr->AddComponent<Pistachio::ParentComponent>();
			auto comp_ptr = &comp;
			T retVal = gcnew T();
			((ParentComponent^)retVal)->m_ptr = comp_ptr;
			return retVal;
		}
		else if (T::typeid == TagComponent::typeid)
		{
			auto& comp = m_ptr->AddComponent<Pistachio::TagComponent>();
			auto comp_ptr = &comp;
			T retVal = gcnew T();
			((TagComponent^)retVal)->m_ptr = comp_ptr;
			return retVal;
		}
		else if (T::typeid == TransformComponent::typeid)
		{
			auto& comp = m_ptr->AddComponent<Pistachio::TransformComponent>();
			auto comp_ptr = &comp;
			T retVal = gcnew T();
			((TransformComponent^)retVal)->m_ptr = comp_ptr;
			return retVal;
		}
		else if (T::typeid == MeshRendererComponent::typeid)
		{
			auto& comp = m_ptr->AddComponent<Pistachio::MeshRendererComponent>();
			auto comp_ptr = &comp;
			T retVal = gcnew T();
			((MeshRendererComponent^)retVal)->m_ptr = comp_ptr;
			return retVal;
		}
		throw gcnew System::ArgumentException(T::typeid->FullName, " is not a valid Pistachio component");
	}
	generic <typename T>
		bool Entity::HasComponent()
	{
		if (T::typeid == IDComponent::typeid)
		{
			return m_ptr->HasComponent<Pistachio::IDComponent>();
		}
		else if (T::typeid == ParentComponent::typeid)
		{
			return m_ptr->HasComponent<Pistachio::ParentComponent>();
		}
		else if (T::typeid == TagComponent::typeid)
		{
			return m_ptr->HasComponent<Pistachio::TagComponent>();
		}
		else if (T::typeid == TransformComponent::typeid)
		{
			return m_ptr->HasComponent<Pistachio::TransformComponent>();
		}
		else if (T::typeid == MeshRendererComponent::typeid)
		{
			return m_ptr->HasComponent<Pistachio::MeshRendererComponent>();
		}
		throw gcnew System::ArgumentException(T::typeid->FullName, " is not a valid Pistachio component");
	}
}
