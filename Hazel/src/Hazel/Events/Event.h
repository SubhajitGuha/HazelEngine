#pragma once

#include "Hazel/Core.h"
#include<string>
#include<functional>

namespace Hazel {

	enum class EventType {
		None = 0,
		WindowClose, WindowResize, WindowFocus, WindowLostFocus, WindoMoved,
		AppTick, AppUpdate, AppRender,
		KeyPressed, KeyReleased,
		MouseButtonPressed, MouseButtonReleased, MouseMoved, MouseScrolled
	};
	enum EventCategory {
		None = 0,
		EventCategoryApplication = BIT(0),
		EventCategoryInput = BIT(1),
		EventCategoryKeyboard = BIT(2),
		EventCategoryMouse = BIT(3),
		EventCategoryMouseButton = BIT(4)
	};

#define EVENT_CLASS_TYPE(type) static EventType GetStaticType(){return EventType::##type;}\
								virtual EventType GetEventType()const override {return GetStaticType();}\
								virtual const char* GetName() const override{return #type;}
#define EVENT_CLASS_CATEGORY(category) virtual int getCategoryFlag()const override{return category;}

	class HAZEL_API Event {
		friend class EventDispatcher;
	public:
		virtual EventType GetEventType()const = 0;
		virtual const char* GetName() const = 0;
		virtual int getCategoryFlag()const = 0;
		virtual std::string ToString() const { return GetName(); }

		inline bool IsInCategory(EventCategory category) {
			return getCategoryFlag() & category; // bitwise and(&) operation to check whether the category belongs to getCategoryflag() 
												//eg if getcategoryflag() is 0110 and category is 1000 the 0110 & 1000 will result 0000 (false)
		}
	protected:
		bool m_Handeled = false;
	};



	class HAZEL_API EventDispatcher {
		template<typename T>
		using EventFn = std::function<bool(T&)>;
	public:
		EventDispatcher(Event& e)
			:m_Event(e) {}

		template<class t>
		bool Dispatch(EventFn<t> fn) {
			if (m_Event.GetEventType() == t::GetStaticType()) {
				m_Event.m_Handeled = func(*(t*)&m_Event);
				return true;
			}
			return false;
		}
	private:
		Event& m_Event;
	};

	inline std::ostream& operator<<(std::ostream& os, const Event& e)//there is a "<<" operator of format "stream << value" defined in ostream.h
																	//which helps to log custom objects in screen. This function just pushes the 																
	{															   //value of tostring() to the output stream
		return os << e.ToString();
	}
	
}