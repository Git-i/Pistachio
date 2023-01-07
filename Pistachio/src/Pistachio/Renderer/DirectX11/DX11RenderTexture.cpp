#include "ptpch.h"
#include "DX11RenderTexture.h"

void DX11RenderTexture::Create(ID3D11ShaderResourceView* texture, int miplevels, ID3D11RenderTargetView** pRTV)
{
    D3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc;
    D3D11_SHADER_RESOURCE_VIEW_DESC SRVDesc;
    ID3D11Device* pDevice;
    ID3D11Resource* renderTargetTexture;
    texture->GetDesc(&SRVDesc);
    texture->GetResource(&renderTargetTexture);
    texture->GetDevice(&pDevice);
  
    renderTargetViewDesc.Format = SRVDesc.Format;
    renderTargetViewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
    renderTargetViewDesc.Texture2D.MipSlice = 0;
    
    pDevice->CreateRenderTargetView(renderTargetTexture, &renderTargetViewDesc, pRTV);
    pDevice->Release();
    renderTargetTexture->Release();
}
void DX11RenderTexture::CreateDepth(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, ID3D11DepthStencilView** pDSV, int width, int height)
{
    ID3D11Texture2D* pDepthStencil;
    D3D11_TEXTURE2D_DESC depthTexDesc = {};
    depthTexDesc.Width = width;
    depthTexDesc.Height = height;
    depthTexDesc.MipLevels = 1;
    depthTexDesc.ArraySize = 1;
    depthTexDesc.Format = DXGI_FORMAT_D32_FLOAT;
    depthTexDesc.SampleDesc.Count = 1;
    depthTexDesc.SampleDesc.Quality = 0;
    depthTexDesc.Usage = D3D11_USAGE_DEFAULT;
    depthTexDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
    (pDevice)->CreateTexture2D(&depthTexDesc, nullptr, &pDepthStencil);
    D3D11_DEPTH_STENCIL_VIEW_DESC dsv = {};
    dsv.Format = DXGI_FORMAT_D32_FLOAT;
    dsv.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
    dsv.Texture2D.MipSlice = 0;
    (pDevice)->CreateDepthStencilView(pDepthStencil, &dsv, pDSV);
    pDepthStencil->Release();
}
void DX11RenderCubeMap::Create(ID3D11ShaderResourceView* texture, int miplevels, ID3D11RenderTargetView** pRTV)
{
    D3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc;
    D3D11_SHADER_RESOURCE_VIEW_DESC SRVDesc;
    ID3D11Device* pDevice;
    ID3D11Resource* renderTargetTexture;
    texture->GetDesc(&SRVDesc);
    texture->GetResource(&renderTargetTexture);
    texture->GetDevice(&pDevice);
    for (int i = 0; i < 6; i++) {
        renderTargetViewDesc.Format = SRVDesc.Format;
        renderTargetViewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DARRAY;
        renderTargetViewDesc.Texture2DArray.MipSlice = 0;
        renderTargetViewDesc.Texture2DArray.FirstArraySlice = i;
        renderTargetViewDesc.Texture2DArray.ArraySize = 1;
        pDevice->CreateRenderTargetView(renderTargetTexture, &renderTargetViewDesc, &(pRTV[i]));
    }
    pDevice->Release();
    renderTargetTexture->Release();
}
void DX11RenderCubeMap::Bind(ID3D11DeviceContext** pCOntext, ID3D11RenderTargetView* pRenderTargetView, ID3D11DepthStencilView* pDSV)
{
    (*pCOntext)->OMSetRenderTargets(1, &pRenderTargetView, pDSV);
}

