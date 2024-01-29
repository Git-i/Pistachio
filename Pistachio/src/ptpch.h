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
#include <unordered_map>
#include <unordered_set>

#if defined(PT_PLATFORM_WINDOWS)
	#include <windows.h>
	#include <d3d11.h>
	#include <dxgi1_3.h>
	#include <d3dcompiler.h>
	#include <DirectXMath.h>
	#include <Xinput.h>
#endif // PT_PLATFORM_WINDOWS

#include "Pistachio/Core/Log.h"
#include "Pistachio/Debug/Instrumentor.h"
#include "Tracy.hpp"

