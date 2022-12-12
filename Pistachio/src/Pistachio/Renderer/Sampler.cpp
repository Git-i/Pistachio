#include "ptpch.h"
#include "Sampler.h"
#include "DirectX11/DX11Texture.h"
namespace Pistachio {
	SamplerState* SamplerState::Create()
	{
		SamplerState* result = new SamplerState;
		result->ImageSamplerState = DX11SamplerState::Create();
		return result;
	}
	void SamplerState::Bind()
	{
		DX11SamplerState::Bind(ImageSamplerState);
	}
	void SamplerState::ShutDown()
	{
		ImageSamplerState->Release();
	}
}