#pragma once
#include"Event.h"

#include<sstream>

namespace Hazel {
	class HAZEL_API MouseButtonEvent : public Event {
	public:
		inline int GetMouseButton()const { return m_MouseButton; }


		std::string ToString()const override {
			std::stringstream ss;
			ss << "Mouse Button Event" << m_MouseButton;
			return ss.str();
		}

		EVENT_CLASS_CATEGORY(EventCategoryMouse | EventCategoryInput)

	protected:

		MouseButtonEvent(int button)
			:m_MouseButton(button){}
		int m_MouseButton;
	};

	class HAZEL_API MouseButtonPressed :public MouseButtonEvent {
	public:
		MouseButtonPressed(int Button,int presscount)
			:MouseButtonEvent(Button),_MouseButtonRepeatCount(presscount){}
		MouseButtonPressed(int Button)
			:MouseButtonEvent(Button){}

		inline int GetMouseButtonRepeatCount()const { return _MouseButtonRepeatCount; }

		std::string ToString()const override {
			std::stringstream ss;
			ss << "Mouse Button Pressed Event" << m_MouseButton;
			return ss.str();
		}

		EVENT_CLASS_TYPE(MouseButtonPressed)
	private:
		int _MouseButtonRepeatCount=0;
	};

	class HAZEL_API MouseButtonReleased :public MouseButtonEvent {
	public:
		MouseButtonReleased(int button)
			:MouseButtonEvent(button){}

		std::string ToString()const override {
			std::stringstream ss;
			ss << "Mouse Button Released Event" << m_MouseButton;
			return ss.str();
		}

		EVENT_CLASS_TYPE(MouseButtonReleased)

	};

	class HAZEL_API MouseScrollEvent :public Event {
	public:
		MouseScrollEvent(float xOffset,float yOffset)
			:m_XOffset(xOffset),m_YOffset(yOffset){}

		inline float GetXOffset()const { return m_XOffset; }
		inline float GetYOffset()const { return m_YOffset; }

		std::string ToString()const override {
			std::stringstream ss;
			ss << "Mouse Scroll Event"<<m_XOffset<<" , "<<m_YOffset;
			return ss.str();
		}

		EVENT_CLASS_CATEGORY(EventCategoryMouse | EventCategoryInput)
		EVENT_CLASS_TYPE(MouseScrolled)
	private:
		float m_XOffset, m_YOffset;
	};

	class HAZEL_API MouseMoveEvent :public Event {
	public:
		MouseMoveEvent(float x,float y)
			:m_MouseX(x),m_MouseY(y){}

		inline float GetMouseX()const { return m_MouseX; }
		inline float GetMouseY()const { return m_MouseY; }
		
		std::string ToString()const override {
			std::stringstream ss;
			ss << "Mouse Move Event" << m_MouseX << " , " << m_MouseY;
			return ss.str();
		}

		EVENT_CLASS_CATEGORY(EventCategoryMouse | EventCategoryInput)
			EVENT_CLASS_TYPE(MouseMoved)
	private:
		float m_MouseX, m_MouseY;
	};
}