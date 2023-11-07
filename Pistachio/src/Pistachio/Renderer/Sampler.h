#pragma once
#include "../Core.h"
#include "RendererID_t.h"
namespace Pistachio {
	enum class TextureAddress {
		Border, Clamp, Mirror, Wrap
	};
	class SamplerState
	{
	public:
		static SamplerState* Create(TextureAddress addressU, TextureAddress addressV, TextureAddress addressW, D3D11_FILTER filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR);
		void Bind(int slot = 0);
		void ShutDown();
		~SamplerState() { ShutDown(); }
	private:
		PlatformRendererID_t ImageSamplerState;
	};
}