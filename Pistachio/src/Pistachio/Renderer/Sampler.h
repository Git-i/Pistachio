#pragma once
#include "../Core.h"
namespace Pistachio {
	class SamplerState
	{
	public:
		static SamplerState* Create();
		void Bind();
		void ShutDown();
	private:
		ID3D11SamplerState* ImageSamplerState;
	};
}