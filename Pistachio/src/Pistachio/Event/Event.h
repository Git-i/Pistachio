#pragma once
#include "Pistachio/Core.h"

namespace Pistachio {

	enum class EventType
	{
		None,
		WindowClose, WindowResize, WindowFocus, WindowLostFocus, WindowMoved,
		AppTick, AppUpdate, AppRender,
		KeyPressed, KeyReleased,
		MouseButtonPressed, MouseButtonReleased, MouseMoved, MouseScrolled,
		GamepadButtonPressed, GamepadPuttonReleased, GamepadJoystickPressed, GamepadJoystickReleased
	};

	enum EventCategory : char
	{
		None = 0,
		EventCategoryApplication = 1,
		EventCategoryKeyboard = 2,
		EventCategoryMouse = 9,
		EventCategoryMouseButton = 3,
		EventCategoryInput = 42,
		EventCategoryGamepad = 35,
		EventCategoryGamepadJoystick = 7
	};

#define EVENT_CLASS_TYPE(type) static EventType GetStaticType() { return EventType::type; }\
							   EventType GetEventType() const override{ return EventType::type; }\
							   const char* GetName() const override{ return "type"; }
#define EVENT_CLASS_CATEGORY(category) char GetCategory() const override { return category; }

	class PISTACHIO_API Event
	{
	public:
		virtual EventType GetEventType() const = 0;
		//Removable Stuff ----------------------------
		virtual const char* GetName() const = 0;
		//-------------------------------------------
		virtual char GetCategory() const = 0;
		virtual std::string ToString() const = 0 { return GetName(); }

		inline bool isInCategory(EventCategory category)
		{
			return (category % GetCategory() == 0);
		}
		bool& Handled = m_handled;
	protected:
		bool m_handled = false;
	};

	class PISTACHIO_API EventDispatcher
	{
	template<typename T>
	using EventFn = std::function<bool(T&)>;
	public:
		EventDispatcher(Event& event)
			: m_event(event) {}

		template <typename T>
		bool Dispactch(EventFn<T> func)
		{
			if (m_event.GetEventType() == T::GetStaticType())
			{
				m_event.m_handled = func(*(T*)&m_event);
				return true;
			}
			return false;
		}
	private:
		Event& m_event;
	};

	inline std::ostream& operator<<(std::ostream& os, const Event& e)
	{
		return os << e.ToString();
	}
}