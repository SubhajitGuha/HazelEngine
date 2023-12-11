#include <memory>
#include "Log.h"
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

#ifdef HZ_DEBUG
	#if HZ_PLATFORM_WINDOWS
		#define HZ_DEBUGBREAK() __debugbreak()
	#else
		#error "This platform is not supported"
	#endif
#define HZ_ENABLE_ASSERTS
#else
#define HZ_DEBUGBREAK()
#endif // HZ_DEBUG

#ifdef HZ_DEBUG
#define HZ_ASSERT(x,...) { if(!(x)) { HAZEL_ERROR("Assertion Failed: {0}", __VA_ARGS__); HZ_DEBUGBREAK(); } }
#else
#define HZ_ASSERT(x, ...)
#endif
#define BIT(x) (1<<x)
#define GAMMA 2.2

//engine texture slots
#define ALBEDO_SLOT 1
#define ROUGHNESS_SLOT 2
#define NORMAL_SLOT 3
#define ENV_SLOT 4
#define IRR_ENV_SLOT 5
#define SHDOW_MAP1 6
#define SHDOW_MAP2 7
#define SHDOW_MAP3 8
#define SHDOW_MAP4 9
#define SSAO_SLOT 10
#define SSAO_BLUR_SLOT 11
#define DEPTH_SLOT 12
#define NOISE_SLOT 13
#define SCENE_TEXTURE_SLOT 14
#define SCENE_DEPTH_SLOT 15
#define ORIGINAL_SCENE_TEXTURE_SLOT 16
#define HEIGHT_MAP_TEXTURE_SLOT 17
#define FOLIAGE_DENSITY_TEXTURE_SLOT 18
#define PERLIN_NOISE_TEXTURE_SLOT 19
#define G_POSITION_TEXTURE_SLOT 20
#define G_COLOR_TEXTURE_SLOT 21
#define G_NORMAL_TEXTURE_SLOT 22
#define G_ROUGHNESS_METALLIC_TEXTURE_SLOT 23
#define LUT_SLOT 24
#define RT_IMAGE_SLOT 25



namespace Hazel {

	template<typename T>
	using ref = std::shared_ptr<T>;

template<typename X>
using scope = std::unique_ptr<X>;

}