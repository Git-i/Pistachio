#pragma once
#include "Pistachio\Core.h"
#include <type_traits>
#include "d3d11.h"
#include "com_ptr.h"
namespace Pistachio
{
	typedef PISTACHIO_API union {  void* ptr; std::uintptr_t val;} RendererID_t;
#ifdef PISTACHIO_RENDER_API_DX11
	typedef ComPtr<IUnknown> PlatformRendererID_t;
#endif // PISTACHIO_RENDER_API_DX11

}
