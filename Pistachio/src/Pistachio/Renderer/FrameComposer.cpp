#include "ptpch.h"
#include "FrameComposer.h"
#include "Pistachio\Core\Window.h"
#include "RendererBase.h"
#include "Pistachio\Core\Application.h"

namespace Pistachio
{
	CompositionMode FrameComposer::mode;
	uint32_t smallestDivisor(uint32_t n)
	{
		if (n % 2 == 0) return 2;
		for (uint32_t i = 3; i * i <= n; i += 2)
		{
			if (n % i == 0) return i;
		}
		return n;
	}
	std::pair<uint32_t, uint32_t> numDivisions(uint32_t width, uint32_t height, uint32_t count)
	{
		uint32_t x = width, y = height;
		for (uint32_t i = 0; i < count; i++)
		{
			uint32_t* longer = (x > y) ? &x : &y;
			uint32_t sub_divs = smallestDivisor(count);
			*longer /= sub_divs;
			count /= sub_divs;
		}
		return { width / x,height / y };
	}
	void FrameComposer::Compose(Scene** scenes, uint32_t count)
	{
		if (Application::Get().IsHeadless()) PT_CORE_ERROR("Cannot Compose On Headless App");
		if (mode == CompositionMode::SimpleCopy)
		{
			unsigned int height = ((WindowData*)GetWindowDataPtr())->height;
			unsigned int width = ((WindowData*)GetWindowDataPtr())->width;
			auto [nx, ny] = numDivisions(width, height, count);
			RHI::Extent3D dstSize = { width / nx, height / ny, 1 };
			for (uint32_t x = 0; x < nx; x++)
			{
				for (uint32_t y = 0; y < ny; y++)
				{
					uint32_t index = x + y * nx;
					RHI::Extent3D srcSize = { scenes[index]->sceneResolution[0], scenes[index]->sceneResolution[1], 1 };
					RendererBase::mainCommandList->BlitTexture(scenes[index]->finalRender.GetID(),
						RendererBase::backBufferTextures[RendererBase::currentRTVindex],
						srcSize, { 0,0,0 },
						dstSize, { (int)(x * dstSize.width), (int)(y * dstSize.height),0 });
				}
			}
		}
	}
	void FrameComposer::SetCompositionMode(CompositionMode _mode)
	{
		mode = _mode;
	}
}
