#include "ptpch.h"
#include "DX11Texture.h"
#include "stb_image.h"
#include "../RendererBase.h"
#include "../../Utils/RendererUtils.h"
namespace Pistachio {
	Error DX11Texture::Create(const char* path, unsigned int* width, unsigned int* height, ID3D11ShaderResourceView** pSRV)
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
        RendererBase::Getd3dDevice()->CreateShaderResourceView(ImageTexture, &srvdesc, pSRV);
        ImageTexture->Release();
        return 0;
	}
	Error DX11Texture::Create(void* data, unsigned int width, unsigned int height, TextureFormat format, ID3D11ShaderResourceView** pSRV)
    {
        HRESULT Hr;
        D3D11_TEXTURE2D_DESC ImageTextureDesc = {};

        ImageTextureDesc.Width = width;
        ImageTextureDesc.Height = height;
        ImageTextureDesc.MipLevels = 1;
        ImageTextureDesc.ArraySize = 1;
        ImageTextureDesc.Format = RendererUtils::DXGITextureFormat(format);
        ImageTextureDesc.SampleDesc.Count = 1;
        ImageTextureDesc.SampleDesc.Quality = 0;
        ImageTextureDesc.Usage = D3D11_USAGE_DEFAULT;
        ImageTextureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
        ImageTextureDesc.CPUAccessFlags = 0;
        ImageTextureDesc.MiscFlags = 0;

        D3D11_SUBRESOURCE_DATA ImageSubresourceData = {};

        ImageSubresourceData.pSysMem = data;
        ImageSubresourceData.SysMemPitch = (width) * 4;

        ID3D11Texture2D* ImageTexture;
        Hr = RendererBase::Getd3dDevice()->CreateTexture2D(&ImageTextureDesc, &ImageSubresourceData, &ImageTexture);
        assert(SUCCEEDED(Hr));
        D3D11_SHADER_RESOURCE_VIEW_DESC srvdesc = {};
        srvdesc.Format = ImageTextureDesc.Format;
        srvdesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
        srvdesc.Texture2D.MostDetailedMip = 0;
        srvdesc.Texture2D.MipLevels = 1;
        RendererBase::Getd3dDevice()->CreateShaderResourceView(ImageTexture, &srvdesc, pSRV);
        ImageTexture->Release();
        return 0;
	}
    Error DX11Texture::CreateFloat(const char* path, unsigned int* width, unsigned int* height, ID3D11ShaderResourceView** pSRV)
    {
        HRESULT Hr;
        //stbi_set_flip_vertically_on_load(true);
        int Width, Height, nrComponents;
        float* data = nullptr;
        if (stbi_is_hdr(path)) {
            data = stbi_loadf(path, &Width, &Height, &nrComponents, 4);
            *width = Width;
            *height = Height;
        }

            D3D11_TEXTURE2D_DESC ImageTextureDesc = {};

            ImageTextureDesc.Width = *width;
            ImageTextureDesc.Height = *height;
            ImageTextureDesc.MipLevels = 1;
            ImageTextureDesc.ArraySize = 1;
            ImageTextureDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
            ImageTextureDesc.SampleDesc.Count = 1;
            ImageTextureDesc.SampleDesc.Quality = 0;
            ImageTextureDesc.Usage = D3D11_USAGE_DEFAULT;
            ImageTextureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
            ImageTextureDesc.CPUAccessFlags = 0;
            ImageTextureDesc.MiscFlags = 0;

            D3D11_SUBRESOURCE_DATA ImageSubresourceData = {};

            ImageSubresourceData.pSysMem = data;
            ImageSubresourceData.SysMemPitch = *(width) * sizeof(float) * 4;

            ID3D11Texture2D* ImageTexture;
            Hr = RendererBase::Getd3dDevice()->CreateTexture2D(&ImageTextureDesc, &ImageSubresourceData, &ImageTexture);
            assert(SUCCEEDED(Hr));
            stbi_image_free(data);
            D3D11_SHADER_RESOURCE_VIEW_DESC srvdesc = {};
            srvdesc.Format = ImageTextureDesc.Format;
            srvdesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
            srvdesc.Texture2D.MostDetailedMip = 0;
            srvdesc.Texture2D.MipLevels = 1;
            RendererBase::Getd3dDevice()->CreateShaderResourceView(ImageTexture, &srvdesc, pSRV);
            ImageTexture->Release();

            return 0;  
    }
    void DX11Texture::Bind(ID3D11ShaderResourceView*const* ppTextureViews, int slot, int numTextures)
    {
        RendererBase::Getd3dDeviceContext()->PSSetShaderResources(slot, numTextures, ppTextureViews);
    }
    Error DX11SamplerState::Create(D3D11_TEXTURE_ADDRESS_MODE addressU, D3D11_TEXTURE_ADDRESS_MODE addressv, D3D11_TEXTURE_ADDRESS_MODE addressw, ID3D11SamplerState** ppSamplerState)
    {
        D3D11_SAMPLER_DESC ImageSamplerDesc = {};
        ImageSamplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
        ImageSamplerDesc.AddressU = addressU;
        ImageSamplerDesc.AddressV = addressv;
        ImageSamplerDesc.AddressW = addressw;
        ImageSamplerDesc.BorderColor[0] = 1.f;
        ImageSamplerDesc.BorderColor[1] = 1.f;
        ImageSamplerDesc.BorderColor[2] = 1.f;
        ImageSamplerDesc.BorderColor[3] = 1.f;
        ImageSamplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
        ImageSamplerDesc.MinLOD = 0;
        ImageSamplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
        RendererBase::Getd3dDevice()->CreateSamplerState(&ImageSamplerDesc, ppSamplerState);
        return Error(ErrorType::Success, std::string(__FUNCTION__));
    }
    void DX11SamplerState::Bind(ID3D11SamplerState* ImageSamplerState, int slot)
    {
        RendererBase::Getd3dDeviceContext()->PSSetSamplers(slot, 1, &ImageSamplerState);
    }
}