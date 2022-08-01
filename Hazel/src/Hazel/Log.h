#pragma once
#include <memory>
#include "Core.h"
#include "spdlog/spdlog.h"

namespace Hazel {

	class HAZEL_API Log {
	public:
		static void init();

		inline static std::shared_ptr<spdlog::logger>& GetCoreLogger() { return s_corelogger; }
		inline static std::shared_ptr<spdlog::logger>& GetAppLogger() { return s_applogger; }
		
	private:
		static std::shared_ptr<spdlog::logger> s_corelogger;
		static std::shared_ptr<spdlog::logger> s_applogger;
	};

}

//core
#define HAZEL_CORE_TRACE(...) Hazel::Log::GetCoreLogger()->trace(__VA_ARGS__)
#define HAZEL_CORE_ERROR(...) Hazel::Log::GetCoreLogger()->error(__VA_ARGS__)
#define HAZEL_CORE_WARN(...) Hazel::Log::GetCoreLogger()->warn(__VA_ARGS__)
#define HAZEL_CORE_INFO(...) Hazel::Log::GetCoreLogger()->info(__VA_ARGS__)
//#define HAZEL_CORE_FETAL(...) Hazel::Log::GetCoreLogger()->fetal(__VA_ARGS__)

//app
#define HAZEL_TRACE(...) Hazel::Log::GetAppLogger()->trace(__VA_ARGS__)
#define HAZEL_ERROR(...) Hazel::Log::GetAppLogger()->error(__VA_ARGS__)
#define HAZEL_WARN(...) Hazel::Log::GetAppLogger()->warn(__VA_ARGS__)
#define HAZEL_INFO(...) Hazel::Log::GetAppLogger()->info(__VA_ARGS__)