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
        if (pTextureView) {
            while (pTextureView->Release()) {};
            pTextureView = NULL;
        }
    }

    void Texture2D::Bind(int slot) const
    {
        this->slot = slot;
        DX11Texture::Bind(&pTextureView, slot);
    }

    Texture2D* Texture2D::Create(const char* path)
    {
        Texture2D* result = new Texture2D;
        result->m_Width = 0;
        result->m_Height = 0;
        DX11Texture::Create(path, &result->m_Width, &result->m_Height, &result->pTextureView);
        return result;
    }
    void Texture2D::CreateStack(const char* path)
    {
        m_Width = 0;
        m_Height = 0;
        DX11Texture::Create(path, &m_Width, &m_Height, &pTextureView);
    }
    unsigned int FloatTexture2D::GetHeight() const
    {
        return 0;
    }
    unsigned int FloatTexture2D::GetWidth() const
    {
        return 0;
    }
    void FloatTexture2D::Bind(int slot) const
    {
        DX11Texture::Bind(&pTextureView, slot);
    }
    FloatTexture2D* FloatTexture2D::Create(const char* path)
    {
        FloatTexture2D* result = new FloatTexture2D;
        DX11Texture::CreateFloat(path, &result->m_Width, &result->m_Height, &result->pTextureView);
        return result;
    }
    void FloatTexture2D::CreateStack(const char* path)
    {
        DX11Texture::CreateFloat(path, &m_Width, &m_Height, &pTextureView);
    }
    FloatTexture2D::~FloatTexture2D()
    {
        if (pTextureView) {
            while (pTextureView->Release()) {};
            pTextureView = NULL;
        }
    }
}