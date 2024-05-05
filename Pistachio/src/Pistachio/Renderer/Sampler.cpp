#include "ptpch.h"
#include "Sampler.h"
#include "DirectX11/DX11Texture.h"
#include "RendererBase.h"
namespace Pistachio {
	const SamplerStateDesc SamplerStateDesc::Default =
	{
		Filter::Linear,
		Filter::Linear,
		Filter::Linear,
		TextureAddress::Clamp,
		TextureAddress::Clamp,
		TextureAddress::Clamp,
		1,
		ComparisonFunc::Never,
		0.f,
		FLT_MIN,
		FLT_MAX,
		{1.f, 1.f, 1.f, 1.f},
		false,
		false
	};
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
		default: return D3D11_TEXTURE_ADDRESS_BORDER;
			break;
		}
	}
	D3D11_FILTER GetFilter(const SamplerStateDesc& stateDesc)
	{
		int minFilter = (int)stateDesc.minFilter;
		int magFilter = (int)stateDesc.magFilter;
		int mipFilter = (int)stateDesc.mipFilter;
		D3D11_FILTER filter;
		if (stateDesc.AnisotropyEnable)
			filter = D3D11_ENCODE_ANISOTROPIC_FILTER(stateDesc.ComparisonEnable ? 1 : 0);
		else
			filter = D3D11_ENCODE_BASIC_FILTER(minFilter, magFilter, mipFilter, stateDesc.ComparisonEnable ? 1 : 0);
		return filter;
	}
	SamplerState* SamplerState::Create(const SamplerStateDesc& stateDesc)
	{
		SamplerState* result = new SamplerState;
		D3D11_SAMPLER_DESC sDesc = {};
		sDesc.AddressU = D3DTextureAddress(stateDesc.AddressU);
		sDesc.AddressV = D3DTextureAddress(stateDesc.AddressV);
		sDesc.AddressW = D3DTextureAddress(stateDesc.AddressW);
		sDesc.BorderColor[0] = stateDesc.BorderColor[0];
		sDesc.BorderColor[1] = stateDesc.BorderColor[1];
		sDesc.BorderColor[2] = stateDesc.BorderColor[2];
		sDesc.BorderColor[3] = stateDesc.BorderColor[3];
		sDesc.ComparisonFunc = (D3D11_COMPARISON_FUNC)stateDesc.func;
		sDesc.Filter = GetFilter(stateDesc);
		sDesc.MipLODBias = stateDesc.MipLODBias;
		sDesc.MinLOD = stateDesc.MinLOD;
		sDesc.MaxLOD = stateDesc.MaxLOD;
		sDesc.MaxAnisotropy = stateDesc.MaxAnisotropy;
		//RendererBase::Getd3dDevice()->CreateSamplerState(&sDesc, (ID3D11SamplerState**)(result->ImageSamplerState.ReleaseAndGetAddressOf()));
		return result;
	}
	void SamplerState::Bind(int slot)
	{
		//RendererBase::Getd3dDeviceContext()->PSSetSamplers(slot, 1, (ID3D11SamplerState*const*)ImageSamplerState.GetAddressOf());
	}
}