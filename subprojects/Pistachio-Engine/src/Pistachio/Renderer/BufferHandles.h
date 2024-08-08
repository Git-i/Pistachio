#pragma once
#include <cstdint>
namespace Pistachio
{
	struct RendererVBHandle
	{
		//do we need to add size too?
		uint32_t handle;
		uint32_t size;
	};
	struct RendererIBHandle
	{
		uint32_t handle;
		uint32_t size;
	};
	struct RendererCBHandle
	{
		uint32_t handle;
		uint32_t actual_size;
		uint32_t size;
	};
}
