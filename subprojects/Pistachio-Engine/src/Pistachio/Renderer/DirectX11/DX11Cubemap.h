#pragma once
#include "../../Core/Error.h"
namespace Pistachio {
	class DX11Cubemap {
	public:
		static Error Create(int Width, int Height, ID3D11Device* pDevice, ID3D11ShaderResourceView** pSRV, ID3D11Texture2D** rendertargettexture,int miplevels);
	};
}