#pragma once
#ifdef HZ_PLATFORM_WINDOWS
#ifdef HZ_DYNAMIC_LINK
	#ifdef HZ_BUILD_DLL
		#define HAZEL_API __declspec(dllexport)
	#else
		#define HAZEL_API __declspec(dllimport)
	#endif // HZ_BUILD_DLL
#else
#define HAZEL_API
#endif 

#else
#error Hazel only supports windows
#endif // HZ_PLATFORM_WINDOWS

#define BIT(x) (1<<x)