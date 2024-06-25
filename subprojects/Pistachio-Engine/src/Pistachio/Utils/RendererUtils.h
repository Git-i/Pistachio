#pragma once
#include <cstdint>
namespace Pistachio {
	
	namespace RendererUtils {
		
		static uint32_t ConstantBufferElementSize(uint32_t byteSize)
		{
			return (byteSize + 255) & ~255;
		}
	}
}