#include "ptpch.h"
#include "DX11Texture.h"
#include "stb_image.h"

#include "../RendererBase.h"
namespace Pistachio {
	ID3D11ShaderResourceView* DX11Texture::Create(const char* path, unsigned int* width, unsigned int* height)
	{
        HRESULT Hr;
        int Width, Height, nChannels;
        stbi_uc* data = stbi_load(path, &Width, &Height, &nChannels, 4);
        *width = Width;
        *height = Height;
        D3D11_TEXTURE2D_DESC ImageTextureDesc = {};

        ImageTextureDesc.Width = *width;
        ImageTextureDesc.Height = *height;
        ImageTextureDesc.MipLevels = 1;
        ImageTextureDesc.ArraySize = 1;
        ImageTextureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        ImageTextureDesc.SampleDesc.Count = 1;
        ImageTextureDesc.SampleDesc.Quality = 0;
        ImageTextureDesc.Usage = D3D11_USAGE_DEFAULT;
        ImageTextureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
        ImageTextureDesc.CPUAccessFlags = 0;
        ImageTextureDesc.MiscFlags = 0;

        D3D11_SUBRESOURCE_DATA ImageSubresourceData = {};

        ImageSubresourceData.pSysMem = data;
        ImageSubresourceData.SysMemPitch = *(width) * 4;

        ID3D11Texture2D* ImageTexture;
        Hr = RendererBase::Getd3dDevice()->CreateTexture2D(&ImageTextureDesc, &ImageSubresourceData, &ImageTexture);
        assert(SUCCEEDED(Hr));
        stbi_image_free(data);
        D3D11_SHADER_RESOURCE_VIEW_DESC srvdesc = {};
        srvdesc.Format = ImageTextureDesc.Format;
        srvdesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
        srvdesc.Texture2D.MostDetailedMip = 0;
        srvdesc.Texture2D.MipLevels = 1;
        ID3D11ShaderResourceView* pTextureView;
        RendererBase::Getd3dDevice()->CreateShaderResourceView(ImageTexture, &srvdesc, &pTextureView);

        return pTextureView;
	}
    void DX11Texture::Bind(ID3D11ShaderResourceView* pTextureView)
    {
        RendererBase::Getd3dDeviceContext()->PSSetShaderResources(0, 1, &pTextureView);
    }
    ID3D11SamplerState* DX11SamplerState::Create()
    {
        D3D11_SAMPLER_DESC ImageSamplerDesc = {};
        ID3D11SamplerState* ImageSamplerState;
        ImageSamplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
        ImageSamplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
        ImageSamplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
        ImageSamplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
        RendererBase::Getd3dDevice()->CreateSamplerState(&ImageSamplerDesc, &ImageSamplerState);
        return ImageSamplerState;
    }
    void DX11SamplerState::Bind(ID3D11SamplerState* ImageSamplerState)
    {
        RendererBase::Getd3dDeviceContext()->PSSetSamplers(0, 1, &ImageSamplerState);
    }
}