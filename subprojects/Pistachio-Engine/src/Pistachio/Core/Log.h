#pragma once
#define SPDLOG_NO_TLS
#include "Pistachio/Core.h"
#include <spdlog/spdlog.h>
#include <spdlog/fmt/ostr.h>

namespace Pistachio {
	
	class PISTACHIO_API Log
	{
	public:
		static void Init();
		static std::shared_ptr<spdlog::logger>& GetCoreLogger();
		static std::shared_ptr<spdlog::logger>& GetClientLogger();
	private:
		static std::shared_ptr<spdlog::logger> s_CoreLogger;
		static std::shared_ptr<spdlog::logger> s_ClientLogger;
	};
}

#if _DEBUG
#define PT_CORE_TRACE(...) Pistachio::Log::GetCoreLogger()->trace(__VA_ARGS__)
#define PT_CORE_INFO(...)  Pistachio::Log::GetCoreLogger()->info(__VA_ARGS__)
#define PT_CORE_WARN(...)  Pistachio::Log::GetCoreLogger()->warn(__VA_ARGS__)
#define PT_CORE_ERROR(...) Pistachio::Log::GetCoreLogger()->error(__VA_ARGS__)
#define PT_DEBUG_REGION(...) __VA_ARGS__

#define PT_TRACE(...)      Pistachio::Log::GetClientLogger()->trace(__VA_ARGS__)
#define PT_INFO(...)       Pistachio::Log::GetClientLogger()->info(__VA_ARGS__)
#define PT_WARN(...)       Pistachio::Log::GetClientLogger()->warn(__VA_ARGS__)
#define PT_ERROR(...)      Pistachio::Log::GetClientLogger()->error(__VA_ARGS__)
#else
#define PT_CORE_TRACE(...)
#define PT_CORE_INFO(...) 
#define PT_CORE_WARN(...) 
#define PT_CORE_ERROR(...)
#define PT_DEBUG_REGION(...)
#define PT_TRACE(...)     
#define PT_INFO(...)      
#define PT_WARN(...)      
#define PT_ERROR(...)     
#endif


