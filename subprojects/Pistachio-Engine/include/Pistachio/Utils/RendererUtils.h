#pragma once
#include <d3d11.h>
namespace Pistachio {
	static enum class TextureFormat {
		RGBA16F = 0, RGBA32F = 2, RGBA8U = 4, INT = 6, R32F = 8,D24S8 = 1, D32F = 3
	};
	namespace RendererUtils {
		static DXGI_FORMAT DXGITextureFormat(Pistachio::TextureFormat format) {
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
	}
}