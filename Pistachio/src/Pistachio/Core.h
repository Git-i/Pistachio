#pragma once

#ifdef PT_PLATFORM_WINDOWS
	#ifdef PISTACHIO_BUILD_DLL
		#define PISTACHIO_API __declspec(dllexport)
	#else
		#define PISTACHIO_API __declspec(dllimport)
	#endif
#else
	#error Pistachio Engine Only Supports Windows
#endif // PT_PLATFROM_WINDOWS

