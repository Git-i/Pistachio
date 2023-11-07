#include "ptpch.h"
#include "DX11Cubemap.h"

#include "../RendererBase.h"
namespace Pistachio {
    Error DX11Cubemap::Create(int Width, int Height, ID3D11Device* pDevice, ID3D11ShaderResourceView** pSRV, ID3D11Texture2D** renderTargetTexture, int miplevels)
    {
        PT_PROFILE_FUNCTION();
        D3D11_TEXTURE2D_DESC textureDesc;
        D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc;

        ZeroMemory(&textureDesc, sizeof(textureDesc));

        textureDesc.Width = Width;
        textureDesc.Height = Height;
        textureDesc.MipLevels = miplevels;
        textureDesc.ArraySize = 6;
        textureDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
        textureDesc.SampleDesc.Count = 1;
        textureDesc.SampleDesc.Quality = 0;
        textureDesc.Usage = D3D11_USAGE_DEFAULT;
        textureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
        textureDesc.CPUAccessFlags = 0;
        textureDesc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE | D3D11_RESOURCE_MISC_GENERATE_MIPS;

        pDevice->CreateTexture2D(&textureDesc, NULL, renderTargetTexture);

        shaderResourceViewDesc.Format = textureDesc.Format;
        shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
        shaderResourceViewDesc.Texture2D.MostDetailedMip = 0;
        shaderResourceViewDesc.Texture2D.MipLevels = miplevels;

        pDevice->CreateShaderResourceView((*renderTargetTexture), &shaderResourceViewDesc, pSRV);
        return Error(ErrorType::Success, std::string(__FUNCTION__));
    }
}

