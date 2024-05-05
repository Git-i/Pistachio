#pragma once
#define PT_CUSTOM_ENTRY
#include "Pistachio.h"
#include "ManagedBase.h"
#include <vcclr.h>
using namespace System;
namespace PistachioCS
{
	public ref class Layer abstract : public ManagedBase<Pistachio::Layer>
	{
	public:
		Layer();
		virtual void OnAttach() {}
		virtual void OnDetach() {}
		virtual void OnUpdate(float delta) {}
		virtual void OnImGuiRender() {}
	};
	class LayerWrapper : public Pistachio::Layer
	{
	public:
		LayerWrapper(PistachioCS::Layer^ owner) : m_owner(owner) {};
		void OnAttach() override
		{
			m_owner->OnAttach();
		}
		void OnDetach() override
		{
			m_owner->OnDetach();
		}
		void OnUpdate(float delta) override
		{
			m_owner->OnUpdate(delta);
		}
		void OnImGuiRender() override
		{
			m_owner->OnImGuiRender();
		}
		gcroot<PistachioCS::Layer^> m_owner;
	};
	
	

}
