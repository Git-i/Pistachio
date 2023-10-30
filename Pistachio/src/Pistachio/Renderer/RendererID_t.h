#pragma once
#include <type_traits>
#include <wrl.h>
namespace Pistachio
{
	typedef union {  void* ptr; std::uintptr_t val;} RendererID_t;
#ifdef PISTACHIO_RENDER_API_DX11
	typedef Microsoft::WRL::ComPtr<IUnknown> PlatformRendererID_t;
#endif // PISTACHIO_RENDER_API_DX11

}
