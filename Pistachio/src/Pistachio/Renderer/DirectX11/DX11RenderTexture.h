#pragma once
class DX11RenderTexture
{
public:
	static void Create(ID3D11ShaderResourceView* texture, int miplevels, ID3D11RenderTargetView** pRTV);
	static void CreateDepth(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, ID3D11DepthStencilView** pDSV, int width, int height);

};
class DX11RenderCubeMap
{
public:
	static void Create(ID3D11ShaderResourceView* texture, int miplevels, ID3D11RenderTargetView** pRTV);
	static void Bind(ID3D11DeviceContext** pCOntext, ID3D11RenderTargetView* pRenderTargetView, ID3D11DepthStencilView* pDSV);
};