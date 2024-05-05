#pragma once
#include "Pistachio/Core.h"
#include <type_traits>
namespace Pistachio
{
	typedef PISTACHIO_API union {  void* ptr; std::uintptr_t val;} RendererID_t;

}
