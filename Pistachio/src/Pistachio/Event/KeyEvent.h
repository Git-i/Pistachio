#pragma once

#include "Event.h"

namespace Pistachio {
	class PISTACHIO_API KeyPressedEvent : public Event
	{
	public:
		KeyPressedEvent(int KeyCode, bool isRepeat = false)
			: m_KeyCode(KeyCode), m_IsRepeat(isRepeat) {}
		inline int GetKeyCode() const { return m_KeyCode; }
		bool isRepeat() { return m_IsRepeat;  }
		std::string ToString() const override
		{
			return "KeyPressedEvent: " + std::to_string(m_KeyCode) + " (repeat = " + std::to_string(m_IsRepeat) + ")";
		}
		EVENT_CLASS_TYPE(KeyPressed)
		EVENT_CLASS_CATEGORY(EventCategoryKeyboard)
	private:
		int m_KeyCode;
		bool m_IsRepeat;
	};
	class PISTACHIO_API KeyReleasedEvent : public Event
	{
	public:
		KeyReleasedEvent(int KeyCode)
			: m_KeyCode(KeyCode) {}
		inline int GetKeyCode() const { return m_KeyCode; }
		
		std::string ToString() const override
		{
			return "KeyReleasedEvent: " + std::to_string(m_KeyCode);
		}
		EVENT_CLASS_TYPE(KeyReleased)
		EVENT_CLASS_CATEGORY(EventCategoryKeyboard)
	private:
		int m_KeyCode;
	};
}
