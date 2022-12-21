#include "ptpch.h"
#include "Texture.h"
#include "RendererBase.h"
#include <stb_image.h>
#include "DirectX11/DX11Texture.h"
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
        pTextureView->Release();
    }

    void Texture2D::Bind(int slot) const
    {
        DX11Texture::Bind(pTextureView, slot);
    }

    Texture2D* Texture2D::Create(const char* path)
    {
        Texture2D* result = new Texture2D;
        result->m_Width = 0;
        result->m_Height = 0;
        result->pTextureView = DX11Texture::Create(path, &result->m_Width, &result->m_Height);
        return result;
    }
    Texture2D Texture2D::CreateStack(const char* path)
    {
        Texture2D result;
        result.m_Width = 0;
        result.m_Height = 0;
        result.pTextureView = DX11Texture::Create(path, &result.m_Width, &result.m_Height);
        return result;
    }
    unsigned int Texture3D::GetHeight() const
    {
        return 0;
    }
    unsigned int Texture3D::GetWidth() const
    {
        return 0;
    }
    void Texture3D::Bind(int slot) const
    {
        DX11Texture::Bind(pTextureView, slot);
    }
    Texture3D* Texture3D::Create(const char* path)
    {
        Texture3D* result = new Texture3D;
        result->pTextureView = DX11Texture::Create3D(path, &result->m_Width, &result->m_Height);
        return result;
    }
    void Texture3D::CreateStack(const char* path)
    {
    }
}