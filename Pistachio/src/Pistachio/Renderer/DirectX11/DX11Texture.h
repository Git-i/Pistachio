#pragma once
namespace Pistachio {
	class DX11Texture
	{
	public:
		static ID3D11ShaderResourceView* Create(const char* path, unsigned int* width, unsigned int* height);
		static ID3D11ShaderResourceView* Create3D(const char* path, unsigned int* width, unsigned int* height);
		static void Bind(ID3D11ShaderResourceView* pTextureView, int slot);
	};
	class DX11SamplerState
	{
	public:
		static ID3D11SamplerState* Create();
		static void Bind(ID3D11SamplerState* ImageSamplerState);
	};
}
