#pragma once
#include "../../Core/Error.h"
namespace Pistachio {
	class DX11Texture
	{
	public:
		static Error Create(const char* path, unsigned int* width, unsigned int* height, ID3D11ShaderResourceView** pSRV);
		static Error Create(void* data, unsigned int width, unsigned int height, ID3D11ShaderResourceView** pSRV);
		static Error CreateFloat(const char* path, unsigned int* width, unsigned int* height, ID3D11ShaderResourceView** pSRV);
		static void Bind(ID3D11ShaderResourceView*const* pTextureView, int slot=0, int numTextures=1);
	};
	class DX11SamplerState
	{
	public:
		static Error Create(D3D11_TEXTURE_ADDRESS_MODE addressU, D3D11_TEXTURE_ADDRESS_MODE addressv, D3D11_TEXTURE_ADDRESS_MODE addressw, ID3D11SamplerState** ppSamplerStaete);
		static void Bind(ID3D11SamplerState* ImageSamplerState);
	};
}
