#include "hzpch.h"
#include"Window.h"
#include "Core.h"

#ifdef HZ_PLATFORM_WINDOWS
#include"platform/WindowsWindow.h"
#endif // HZ

namespace Hazel {
	Window* Window::Create(const WindowProp& prop)
	{
		#ifdef HZ_PLATFORM_WINDOWS
			return new WindowsWindow();
		#endif // HZ
		return nullptr;
	}
}