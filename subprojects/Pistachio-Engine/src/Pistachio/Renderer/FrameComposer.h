#pragma once
#include "../Scene/Scene.h"
namespace Pistachio
{
	enum class CompositionMode
	{
		SimpleCopy = 0,
		Script = 1
	};
	//purely static class that interacts with swapchain images directly
	class PISTACHIO_API FrameComposer
	{
	public:
		static void Compose(Scene** scenes, uint32_t count);
		static void SetCompositionMode(CompositionMode mode);
		static CompositionMode mode;
	};
}
