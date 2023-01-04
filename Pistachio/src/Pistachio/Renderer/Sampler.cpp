#include "ptpch.h"
#include "Sampler.h"
#include "DirectX11/DX11Texture.h"
namespace Pistachio {
	
	D3D11_TEXTURE_ADDRESS_MODE D3DTextureAddress(TextureAddress address)
	{
		switch (address)
		{
		case Pistachio::TextureAddress::Border: return D3D11_TEXTURE_ADDRESS_BORDER;
			break;
		case Pistachio::TextureAddress::Clamp: return D3D11_TEXTURE_ADDRESS_CLAMP;
			break;
		case Pistachio::TextureAddress::Mirror: return D3D11_TEXTURE_ADDRESS_MIRROR;
			break;
		case Pistachio::TextureAddress::Wrap: return D3D11_TEXTURE_ADDRESS_WRAP;
			break;
		default:
			break;
		}
	}
	SamplerState* SamplerState::Create(TextureAddress addressU, TextureAddress addressV, TextureAddress addressW)
	{
		SamplerState* result = new SamplerState;
		DX11SamplerState::Create(D3DTextureAddress(addressU), D3DTextureAddress(addressV), D3DTextureAddress(addressW), &result->ImageSamplerState);
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