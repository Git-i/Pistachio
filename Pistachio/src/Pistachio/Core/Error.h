#pragma once
#include "ptpch.h"
#include "Log.h"
#include <filesystem>
namespace Pistachio {
	enum class ErrorType
	{
		Success,Unknown, NullError, NonExistentFile
	};
	class Error {
	public:
		Error(int a) {
			type = (ErrorType)(a);
			severity = GetErrorSeverity(type);
			ReporterString = "Unknown Internal Error";
		}
		inline static bool CheckFileExistence(const char* filepath) {
			if (std::filesystem::exists(filepath))
				return true;
			return false;
		}
		Error(ErrorType etype, const std::string& funcsig) :type(etype),ReporterString(funcsig), severity(GetErrorSeverity(etype)){ };
		inline static std::string GetErrorString(const Error& e) {
			switch (e.GetErrorType())
			{
			case Pistachio::ErrorType::Unknown:
				return "Unkown Internal Error";
				break;
			case Pistachio::ErrorType::NonExistentFile:
				return (std::string("The file passed into the function doesn't exist: ") + std::string(e.GetReporterString()));
				break;
			case Pistachio::ErrorType::NullError:
				return (std::string("A NULL or nullptr parameter was passed into the function: ") + std::string(e.GetReporterString()));
				break;
			default: return "Unregistered Error Type";
				break;
			}
		};
		inline static void LogErrorToConsole(const Error & e) {
			if (e.GetSeverity() == 1)
				PT_CORE_WARN(GetErrorString(e));
			if (e.GetSeverity() == 2)
				PT_CORE_ERROR(GetErrorString(e));
		};
		inline static void AssertOnError(const Error & e) {
			if(e.GetSeverity() == 2)
				PT_CORE_ASSERT(GetErrorString(e) == "0");
		};
		ErrorType GetErrorType() const { return type; };
		const char* GetReporterString() const { return ReporterString.c_str(); };
		static int GetErrorSeverity(ErrorType type){
			switch (type)
			{
			case Pistachio::ErrorType::Unknown: return 2;
				break;
			case Pistachio::ErrorType::Success: return 0;
				break;
			case Pistachio::ErrorType::NullError: return 2;
				break;
			case Pistachio::ErrorType::NonExistentFile: return 2;
				break;
			default: return 0;
				break;
			}
		}
		int GetSeverity() const { return severity; };
	private:
		ErrorType type;
		std::string ReporterString;
		int severity;
	};
}
class Reporter {
	Reporter(std::string& caller);
};
