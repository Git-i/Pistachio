#pragma once
#include "../Core.h"
#include "RendererID_t.h"
#include "../Utils/RendererUtils.h"
#include "../Asset/RefCountedObject.h"
namespace Pistachio {
	class Texture : public RefCountedObject
	{
	public:
		virtual unsigned int GetHeight() const = 0;
		virtual unsigned int GetWidth() const = 0;
	};
	class Texture2D : public Texture
	{
	public:
		unsigned int GetHeight() const override;
		unsigned int GetWidth() const override;
		Texture2D() : m_ID(0), m_Width(0), m_Height(0), m_format(TextureFormat::RGBA8U){};
		void Bind(int slot = 0) const;
		static Texture2D* Create(const char* path, TextureFormat format = TextureFormat::RGBA8U, TextureFlags flags = TextureFlags::NONE);
		static Texture2D* Create(int width, int height, TextureFormat format,void* data, TextureFlags flags = TextureFlags::NONE);
		void CreateStack(const char* path, TextureFormat format, TextureFlags flags = TextureFlags::NONE);
		void CreateStack(int width, int height, TextureFormat format,void* data, TextureFlags flags = TextureFlags::NONE);
		void CopyIntoRegion(Texture2D& source, unsigned int location_x, unsigned int location_y, unsigned int src_left, unsigned int src_right, unsigned int src_up, unsigned int src_down, unsigned int mipSlice = 0, unsigned int arraySlice = 0);
		void CopyInto(Texture2D& source);
		RendererID_t GetID() const;
		//TODO: Asset Management
		bool operator==(const Texture2D& texture) const;
		friend class RenderTexture;
		friend class RenderCubeMap;
	private:
		/*
		 * m_ID = CPU/GPU address to the resource (DirectX 12)
		 * m_ID = View related to the resource (DirectX 11)
		*/
		PlatformRendererID_t m_ID;
		unsigned int m_Width, m_Height, m_MipLevels;
		TextureFormat m_format;
		bool m_bHasView = true;
	private:
		void CreateTexture(void* data, TextureFlags flags);
	};
}

