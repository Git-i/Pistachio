#pragma once
#include <string>

namespace Pistachio {
	class FileDialogs {
	public:
		static std::string SaveFile(const char* filter);
		static std::string OpenFile(const char* filter);
	};
}
