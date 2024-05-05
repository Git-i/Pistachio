#include "ptpch.h"

#include "Log.h"
#include <spdlog/sinks/stdout_color_sinks.h>
namespace Pistachio
{
	std::shared_ptr<spdlog::logger> Log::s_CoreLogger;
	std::shared_ptr<spdlog::logger> Log::s_ClientLogger;
	void Log::Init()
	{
		spdlog::set_pattern("%^[%T] %n: %v%$");
		s_CoreLogger = spdlog::stdout_color_mt("Pistachio");
		s_CoreLogger->set_level(spdlog::level::trace);
		PT_CORE_INFO("Core Logger Initialized");

		s_ClientLogger = spdlog::stdout_color_mt("Application");
		s_ClientLogger->set_level(spdlog::level::trace);
		PT_CORE_INFO("Client Logger Initialized");
	}
	std::shared_ptr<spdlog::logger>& Log::GetCoreLogger() { return s_CoreLogger; }
	std::shared_ptr<spdlog::logger>& Log::GetClientLogger() { return s_ClientLogger; }
}