#pragma once

#include "Event.h"

namespace Pistachio {
	class Entity;
	class EntityCreatedEvent : public Event
	{
	public:
		EntityCreatedEvent(Entity e);

		Entity GetAffectedEntity() const;

		std::string ToString() const override
		{
			return "EntityCreatedEvent: ";
		}

		EVENT_CLASS_TYPE(EntityCreated)
		EVENT_CLASS_CATEGORY(EventCategorySceneGraph)
	private:
		uint32_t entity_ID; //todo handle this better
	};

	
}