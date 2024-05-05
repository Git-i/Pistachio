#pragma once
#define PT_CUSTOM_ENTRY
#include "Pistachio.h"
#include "Components.h"
#include "ManagedBase.h"

namespace PistachioCS
{
	public ref class Entity : public ManagedBase<Pistachio::Entity>
	{
	internal:
		Entity(entt::entity handle, Pistachio::Scene* scene);
	public:
		generic<typename T>  where T : gcnew() T GetComponent();
		generic<typename T>  where T : gcnew() T AddComponent();
		generic<typename T> bool HasComponent();
		bool IsValid();
		
		bool operator==(Entity^ other);
	};
}

