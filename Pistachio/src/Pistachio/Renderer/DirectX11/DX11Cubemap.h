#pragma once
class DX11Cubemap {
public:
	static ID3D11Texture2D* Create(int Width, int Height, ID3D11Device* pDevice, ID3D11ShaderResourceView** pSRV, int miplevels);
	static void Bind(ID3D11DeviceContext** pCOntext, ID3D11RenderTargetView* pRenderTargetView, ID3D11DepthStencilView* pDSV);
};