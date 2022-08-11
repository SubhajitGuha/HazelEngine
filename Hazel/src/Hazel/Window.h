#pragma once
#include "hzpch.h"
#include "Hazel/Core.h"
#include "Events/Event.h"


namespace Hazel{
	struct WindowProp {
		std::string Title;
		unsigned int width, height;
		WindowProp(std::string s="Hazel Engine",unsigned int h=1280,unsigned int w=1280)
			:Title(s),width(w),height(h)
		{}
	};
	//window interface for windows
class HAZEL_API Window {
public:
	using EventCallbackFunc = std::function<void(Event&)>;
	virtual ~Window() {}
	virtual void OnUpdate() = 0;
	virtual unsigned int GetWidth() = 0;
	virtual unsigned int GetHeight() = 0;

	//window parameters
	virtual void SetCallbackEvent(const EventCallbackFunc&) = 0;
	virtual void SetVsync(bool enable) = 0;
	virtual bool b_Vsync()const = 0;
	virtual void* GetNativeWindow() = 0;//gets the GLFWwindow pointer

	static Window* Create(const WindowProp& prop = WindowProp());
};
}