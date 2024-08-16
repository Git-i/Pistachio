#pragma once
// Pistachio Precompiled Headers -----------------------------------------------------------
#define NOMINMAX
#include <iostream>
#include <functional>
#include <memory>
#include <utility>
#include <bitset>
#include <utility>
#include <string>
#include <vector>
#include <array>
#include <chrono>
#include <unordered_map>
#include <unordered_set>

#if defined(PT_PLATFORM_WINDOWS)
	#include <windows.h>
	#include <d3d11.h>
	#include <dxgi1_3.h>
	#include <d3dcompiler.h>
	#include <DirectXMath.h>
	#include <Xinput.h>
#elif defined(PT_PLATFORM_LINUX)
	#include <GLFW/glfw3.h>
	#include <GLFW/glfw3native.h>
#endif // PT_PLATFORM_WINDOWS

# if defined ( __clang__ ) || defined ( __GNUC__ )
# define TracyFunction __PRETTY_FUNCTION__
# elif defined ( _MSC_VER )
# define TracyFunction __FUNCSIG__
# endif
#include "Pistachio/Core/Log.h"
#include "Pistachio/Debug/Instrumentor.h"
#include "tracy/Tracy.hpp"
#include "TraceRHI.h"

