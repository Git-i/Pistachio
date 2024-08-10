#include "ptpch.h"
#include "spdlog/common.h"
#include "spdlog/logger.h"
#include "spdlog/sinks/basic_file_sink.h"
#include "spdlog/spdlog.h"

#include "Log.h"
#include <chrono>
#include <spdlog/sinks/stdout_color_sinks.h>
namespace Pistachio
{
	std::shared_ptr<spdlog::logger> Log::s_CoreLogger;
	std::shared_ptr<spdlog::logger> Log::s_ClientLogger;
	void Log::Init(const char* filename)
	{
		spdlog::set_pattern("%^[%T] %n: %v%$");
		if(!filename)
			s_CoreLogger = spdlog::stdout_color_mt("Pistachio");
		else
		 	s_CoreLogger = spdlog::basic_logger_mt("Pistachio", filename);
		s_CoreLogger->set_level(spdlog::level::trace);
		PT_CORE_INFO("Core Logger Initialized");

		if(!filename)
			s_ClientLogger = spdlog::stdout_color_mt("Application");
		else
			s_ClientLogger = spdlog::basic_logger_mt("Application", filename);
		s_ClientLogger->set_level(spdlog::level::trace);
		PT_CORE_INFO("Client Logger Initialized");
		spdlog::flush_on(spdlog::level::trace);
	}
	std::shared_ptr<spdlog::logger>& Log::GetCoreLogger() { return s_CoreLogger; }
	std::shared_ptr<spdlog::logger>& Log::GetClientLogger() { return s_ClientLogger; }
}