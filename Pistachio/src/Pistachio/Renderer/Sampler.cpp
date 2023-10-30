#include "ptpch.h"
#include "Sampler.h"
#include "DirectX11/DX11Texture.h"
#include "RendererBase.h"
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
		D3D11_SAMPLER_DESC sDesc = {};
		sDesc.AddressU = D3DTextureAddress(addressU);
		sDesc.AddressV = D3DTextureAddress(addressV);
		sDesc.AddressW = D3DTextureAddress(addressW);
		sDesc.BorderColor[0] = 1.f;
		sDesc.BorderColor[1] = 1.f;
		sDesc.BorderColor[2] = 1.f;
		sDesc.BorderColor[3] = 1.f;
		sDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
		sDesc.Filter = D3D11_FILTER_ANISOTROPIC;
		RendererBase::Getd3dDevice()->CreateSamplerState(&sDesc, (ID3D11SamplerState**)(result->ImageSamplerState.ReleaseAndGetAddressOf()));
		return result;
	}
	void SamplerState::Bind(int slot)
	{
		RendererBase::Getd3dDeviceContext()->PSSetSamplers(slot, 1, (ID3D11SamplerState*const*)ImageSamplerState.GetAddressOf());
	}
	void SamplerState::ShutDown()
	{
	}
}