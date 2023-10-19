#include "ptpch.h"
#include "Texture.h"
#include "RendererBase.h"
#include <stb_image.h>
#include "DirectX11/DX11Texture.h"
#include "../Utils/RendererUtils.h"
namespace Pistachio {
    unsigned int Texture2D::GetHeight() const
    {
        return m_Height;
    }

    unsigned int Texture2D::GetWidth() const
    {
        return m_Width;
    }

    Texture2D::~Texture2D()
    {
    }

    void Texture2D::Bind(int slot) const
    {
        PT_PROFILE_FUNCTION();
        DX11Texture::Bind(m_ID.GetAddressOf(), slot);
    }

    Texture2D* Texture2D::Create(const char* path, TextureFormat format)
    {
        PT_PROFILE_FUNCTION()
        Texture2D* result = new Texture2D;
        result->CreateStack(path, format);
        return result;
    }
    void Texture2D::CreateStack(const char* path, TextureFormat format)
    {
        PT_PROFILE_FUNCTION()
        HRESULT Hr;
        int Width, Height, nChannels;
        void* data;
        if (format == TextureFormat::RGBA16F || format == TextureFormat::RGBA32F)
        {
            data = stbi_loadf(path, &Width, &Height, &nChannels, 4);
        }
        else
        {
            data = stbi_load(path, &Width, &Height, &nChannels, 4);
        }
        m_Width = Width;
        m_Height = Height;
        DX11Texture::Create(data, m_Width, m_Height, format, &m_ID);
    }
    void Texture2D::CreateStack(int width, int height, TextureFormat format, void* data)
    {
        m_Width = width;
        m_Height = height;
        DX11Texture::Create(data, width, width, format, &m_ID);
    }
    Texture2D* Texture2D::Create(int width, int height, TextureFormat format,void* data)
    {
        PT_PROFILE_FUNCTION()
        Texture2D* result = new Texture2D;
        result->m_Width = width;
        result->m_Height = height;
        DX11Texture::Create(data, width, width, format, &result->m_ID);
        return result;
    }
}