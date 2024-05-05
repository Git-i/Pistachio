#pragma once
#include "Entity.h"
namespace Pistachio {
	class ScriptableComponent {
	public:
		template<typename T>
		T& GetComponent()
		{
			return m_Entity.GetComponent<T>();
		}
	private:
		Entity m_Entity;
		friend class Scene;
	};
}
