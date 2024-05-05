#pragma once
#include "../Core.h"
namespace Pistachio {
	enum class TextureAddress {
		Border, Clamp, Mirror, Wrap
	};
	class SamplerState
	{
	public:
		static SamplerState* Create(TextureAddress addressU, TextureAddress addressV, TextureAddress addressW);
		void Bind(int slot = 0);
		void ShutDown();
		~SamplerState() { ShutDown(); }
	private:
		ID3D11SamplerState* ImageSamplerState;
	};
}