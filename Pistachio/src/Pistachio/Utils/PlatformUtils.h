#pragma once
#include <string>

namespace Pistachio {
	class PISTACHIO_API FileDialogs {
	public:
		static std::string SaveFile(const char* filter);
		static std::string OpenFile(const char* filter);
	};
}
