
#include "ptpch.h"
#include "../Texture.h"
#include "../RendererBase.h"
#include <stb_image.h>
#include "../../Utils/RendererUtils.h"
namespace Pistachio {
    void Texture2D::CreateTexture(void* data,TextureFlags flags)
    {
        PT_PROFILE_FUNCTION();
        HRESULT Hr;
        D3D11_TEXTURE2D_DESC ImageTextureDesc = {};
        m_MipLevels = 1;
        ImageTextureDesc.Width = m_Width;
        ImageTextureDesc.Height = m_Height;
        ImageTextureDesc.MipLevels = 1;
        ImageTextureDesc.ArraySize = 1;
        ImageTextureDesc.Format = RendererUtils::DXGITextureFormat(m_format);
        ImageTextureDesc.SampleDesc.Count = 1;
        ImageTextureDesc.SampleDesc.Quality = 0;
        ImageTextureDesc.Usage = ((int)flags & (int)TextureFlags::USAGE_STAGING) ? D3D11_USAGE_STAGING : D3D11_USAGE_DEFAULT;
        ImageTextureDesc.BindFlags = ((int)flags & (int)TextureFlags::USAGE_STAGING) ? 0 : D3D11_BIND_SHADER_RESOURCE;
        ImageTextureDesc.CPUAccessFlags = ((unsigned int)flags & (unsigned int)TextureFlags::ALLOW_CPU_ACCESS_READ ) ? D3D11_CPU_ACCESS_READ | D3D11_CPU_ACCESS_WRITE: 0;
        ImageTextureDesc.MiscFlags = 0;

        D3D11_SUBRESOURCE_DATA ImageSubresourceData = {};

        ImageSubresourceData.pSysMem = data;
        ImageSubresourceData.SysMemPitch = (m_Width)*RendererUtils::TextureFormatBytesPerPixel(m_format);
        ID3D11Texture2D* ImageTexture;
        if(data)
            Hr = RendererBase::Getd3dDevice()->CreateTexture2D(&ImageTextureDesc, &ImageSubresourceData, &ImageTexture);
        else
            Hr = RendererBase::Getd3dDevice()->CreateTexture2D(&ImageTextureDesc, nullptr, &ImageTexture);
        assert(SUCCEEDED(Hr));
        if (!(  ((int)flags & (int)TextureFlags::NO_SHADER_USAGE) || ((int)flags & (int)TextureFlags::USAGE_STAGING)  )) 
        {
            D3D11_SHADER_RESOURCE_VIEW_DESC srvdesc = {};
            srvdesc.Format = ImageTextureDesc.Format;
            srvdesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
            srvdesc.Texture2D.MostDetailedMip = 0;
            srvdesc.Texture2D.MipLevels = 1;
            RendererBase::Getd3dDevice()->CreateShaderResourceView(ImageTexture, &srvdesc, (ID3D11ShaderResourceView**)m_ID.ReleaseAndGetAddressOf());
        }
        else
        {
            m_bHasView = false;
            m_ID = ImageTexture;
        }
        ImageTexture->Release();
    }
    unsigned int Texture2D::GetHeight() const
    {
        PT_PROFILE_FUNCTION();
        return m_Height;
    }

    unsigned int Texture2D::GetWidth() const
    {
        PT_PROFILE_FUNCTION();
        return m_Width;
    }

    void Texture2D::Bind(int slot) const
    {
        PT_PROFILE_FUNCTION();
        RendererBase::Getd3dDeviceContext()->PSSetShaderResources(slot, 1, (ID3D11ShaderResourceView*const*)m_ID.GetAddressOf());
    }

    Texture2D* Texture2D::Create(const char* path, TextureFormat format, TextureFlags flags)
    {
        PT_PROFILE_FUNCTION()
            Texture2D* result = new Texture2D;
        result->CreateStack(path, format);
        return result;
    }
    void Texture2D::CreateStack(const char* path, TextureFormat format, TextureFlags flags)
    {
        PT_PROFILE_FUNCTION()
        HRESULT Hr;
        int Width, Height, nChannels;
        void* data;
        if (format == TextureFormat::RGBA16F || format == TextureFormat::RGBA32F)
        {
            PT_PROFILE_SCOPE("stbi_loadf");
            data = stbi_loadf(path, &Width, &Height, &nChannels, 4);
            PT_CORE_ASSERT(data);
        }
        else
        {
            PT_PROFILE_SCOPE("stbi_load");
            data = stbi_load(path, &Width, &Height, &nChannels, 4);
            PT_CORE_ASSERT(data);
        }
        m_Width = Width;
        m_Height = Height;
        m_format = format;
        CreateTexture(data, flags);
        stbi_image_free(data);
    }
    void Texture2D::CreateStack(int width, int height, TextureFormat format, void* data, TextureFlags flags)
    {
        PT_PROFILE_FUNCTION();
        m_Width = width;
        m_Height = height;
        m_format = format;
        CreateTexture(data, flags);
    }
    void Texture2D::CopyIntoRegion(Texture2D& source, unsigned int location_x, unsigned int location_y, unsigned int src_left, unsigned int src_right, unsigned int src_up, unsigned int src_down, unsigned int mipSlice, unsigned int arraySlice)
    {
        PT_PROFILE_FUNCTION();
        ID3D11Resource* pDstResource;
        ID3D11Resource* pSrcResource;
        if (m_bHasView)
            ((ID3D11ShaderResourceView*)m_ID.Get())->GetResource(&pDstResource);
        else
            pDstResource = (ID3D11Resource*)(m_ID.Get());
        ((ID3D11ShaderResourceView*)source.m_ID.Get())->GetResource(&pSrcResource);
        D3D11_BOX sourceRegion;
        sourceRegion.left = src_left;
        sourceRegion.right = src_right;
        sourceRegion.top = src_up;
        sourceRegion.bottom = src_down;
        sourceRegion.front = 0;
        sourceRegion.back = 1;
        RendererBase::Getd3dDeviceContext()->CopySubresourceRegion(pDstResource, D3D11CalcSubresource(mipSlice, arraySlice, m_MipLevels), location_x, location_y, 0, pSrcResource, 0, &sourceRegion);
        if(m_bHasView)
            pDstResource->Release();
        pSrcResource->Release();
    }
    void Texture2D::CopyInto(Texture2D& source)
    {
        PT_PROFILE_FUNCTION();
        ID3D11Resource* pDstResource;
        ID3D11Resource* pSrcResource;
        if (m_bHasView)
            ((ID3D11ShaderResourceView*)m_ID.Get())->GetResource(&pDstResource);
        else
            pDstResource = (ID3D11Resource*)(m_ID.Get());
        ((ID3D11ShaderResourceView*)source.m_ID.Get())->GetResource(&pSrcResource);
        RendererBase::Getd3dDeviceContext()->CopyResource(pDstResource, pSrcResource);
        if(m_bHasView)
            pDstResource->Release();
        pSrcResource->Release();
    }
    void Texture2D::CopyToCPUBuffer(void* buffer)
    {
        //todo rework this function
        Texture2D cptex;
        cptex.CreateStack(1280, 720, TextureFormat::RGBA8U, nullptr, (TextureFlags)((unsigned int)TextureFlags::ALLOW_CPU_ACCESS_READ | (unsigned int)TextureFlags::USAGE_STAGING));
        cptex.CopyInto(*this);
        D3D11_MAPPED_SUBRESOURCE sr;
        ZeroMemory(&sr, sizeof(D3D11_MAPPED_SUBRESOURCE));
        D3D11_TEXTURE2D_DESC desc;
        ID3D11Texture2D* tex = (ID3D11Texture2D*)cptex.m_ID.Get();
        tex->GetDesc(&desc);
        HRESULT hr = Pistachio::RendererBase::Getd3dDeviceContext()->Map(tex, 0, D3D11_MAP_READ, 0, &sr);
        memcpy(buffer, sr.pData, m_Height * m_Width * RendererUtils::TextureFormatBytesPerPixel(m_format));
        Pistachio::RendererBase::Getd3dDeviceContext()->Unmap((ID3D11Texture2D*)tex, 0);
        tex->Release();
    }
    Texture2D* Texture2D::Create(int width, int height, TextureFormat format, void* data, TextureFlags flags)
    {
        PT_PROFILE_FUNCTION()
            Texture2D* result = new Texture2D;
        result->m_Width = width;
        result->m_Height = height;
        result->CreateStack(width, height, format, data, flags);
        return result;
    }
    RendererID_t Texture2D::GetID() const
    {
        PT_PROFILE_FUNCTION();
        RendererID_t ID;
        ID.ptr = m_ID.Get();
        return ID;
    }
    bool Texture2D::operator==(const Texture2D& texture) const
    {
        PT_PROFILE_FUNCTION();
        return (m_ID.Get() == texture.m_ID.Get());
    }
}