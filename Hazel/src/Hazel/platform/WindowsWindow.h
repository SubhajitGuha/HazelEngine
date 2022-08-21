#pragma once
#include "hzpch.h"
#include"Hazel/Window.h"
#include "Hazel/Core.h"
#include "GLFW/glfw3.h"
#include "Hazel/platform/Opengl/OpenGlRenderer.h"

namespace Hazel {
	class WindowsWindow :public Window
	{
	public:
		WindowsWindow();
		~WindowsWindow();
		
		void OnUpdate() override;
		unsigned int GetWidth() override { return m_Data.width; }
		unsigned int GetHeight()override {
			return m_Data.height;
		}
		void SetCallbackEvent(const EventCallbackFunc& callback)override { m_Data.Callbackfunc = callback; }
		void SetVsync(bool enable) override;
		bool b_Vsync()const override;

		inline void* GetNativeWindow() { return m_window; }

		struct WindowData{
			unsigned int width, height;
			std::string name;
			EventCallbackFunc Callbackfunc;
		};

	private:
		GLFWwindow* m_window;
		bool m_isInitilized = false;
		void Init(WindowProp& prop);
		void ShutDown();
		WindowData m_Data;
		WindowProp prop=WindowProp();
		GraphicsContext* Context;
	};
}
