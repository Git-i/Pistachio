#pragma once
#include "Pistachio/Core.h"
#include "Pistachio/Event/Event.h"

namespace Pistachio {
	class PISTACHIO_API Layer {
	public:
		Layer(const char* name = "Layer");
		virtual ~Layer();

		virtual void OnAttach() {}
		virtual void OnDetach() {}
		virtual void OnUpdate() {}
		virtual void OnEvent(Event& event) {}

		inline const char* GetName() const { return m_DebugName; }
	protected:
		const char* m_DebugName;
	};
}



