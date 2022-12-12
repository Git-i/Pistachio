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

    void Texture2D::Bind() const
    {
        DX11Texture::Bind(pTextureView);
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
}