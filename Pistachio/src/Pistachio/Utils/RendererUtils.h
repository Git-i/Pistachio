#pragma once
#include <d3d11.h>
namespace Pistachio {
	enum class TextureFormat {
		RGBA16F = 0, RGBA32F = 2, RGBA8U = 4, INT = 6, R32F = 8, R11G11B10 = 10,D24S8 = 1, D32F = 3
	};
	
	namespace RendererUtils {
		static DXGI_FORMAT DXGITextureFormat(Pistachio::TextureFormat format) 
		{
			switch (format)
			{
			case Pistachio::TextureFormat::RGBA16F: return DXGI_FORMAT_R16G16B16A16_FLOAT;
				break;
			case Pistachio::TextureFormat::RGBA32F: return DXGI_FORMAT_R32G32B32A32_FLOAT;
				break;
			case Pistachio::TextureFormat::RGBA8U: return DXGI_FORMAT_R8G8B8A8_UNORM;
				break;
			case Pistachio::TextureFormat::D32F: return DXGI_FORMAT_D32_FLOAT;
				break;
			case Pistachio::TextureFormat::R11G11B10: return DXGI_FORMAT_R11G11B10_FLOAT;
				break;
			case Pistachio::TextureFormat::D24S8: return DXGI_FORMAT_D24_UNORM_S8_UINT;
				break;
			case Pistachio::TextureFormat::INT: return DXGI_FORMAT_R32_SINT;
				break;
			case Pistachio::TextureFormat::R32F: return DXGI_FORMAT_R32_FLOAT;
				break;
			default: return DXGI_FORMAT_UNKNOWN;
				break;
			}
		}
		static unsigned int TextureFormatBytesPerPixel(Pistachio::TextureFormat format)
		{
			switch (format)
			{
			case Pistachio::TextureFormat::RGBA16F: return 8;
				break;
			case Pistachio::TextureFormat::RGBA32F: return 16;
				break;
			case Pistachio::TextureFormat::RGBA8U: return 4;
				break;
			case Pistachio::TextureFormat::D32F: return 4;
				break;
			case Pistachio::TextureFormat::R11G11B10: return 4;
				break;
			case Pistachio::TextureFormat::D24S8: return 4;
				break;
			case Pistachio::TextureFormat::INT: return 4;
				break;
			case Pistachio::TextureFormat::R32F: return 4;
				break;
			default: return DXGI_FORMAT_UNKNOWN;
				break;
			}
		}
		static uint32_t ConstantBufferElementSize(uint32_t byteSize)
		{
			return (byteSize + 255) & ~255;
		}
	}
}