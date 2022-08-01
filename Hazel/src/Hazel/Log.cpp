#include "Log.h"
#include "spdlog/sinks/stdout_color_sinks.h"

namespace Hazel {
	std::shared_ptr<spdlog::logger> Log::s_corelogger;//you have to make a nonstatic copy of the same variable 
	std::shared_ptr<spdlog::logger> Log::s_applogger;//so that it can be used else where
	void Log::init()
	{
		spdlog::set_pattern("%^[%T] %n: %v%$");
		s_corelogger = spdlog::stdout_color_mt("HAZEL");
		s_corelogger->set_level(spdlog::level::trace);

		s_applogger = spdlog::stdout_color_mt("APP");
		s_applogger->set_level(spdlog::level::trace);
	}
}