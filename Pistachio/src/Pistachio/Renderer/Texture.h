#pragma once
#include "../Core.h"
#include "RendererID_t.h"
#include "../Utils/RendererUtils.h"
namespace Pistachio {
	class Texture
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
		~Texture2D();
		void Bind(int slot = 0) const;
		static Texture2D* Create(const char* path, TextureFormat format = TextureFormat::RGBA8U);
		static Texture2D* Create(int width, int height, TextureFormat format,void* data);
		void CreateStack(const char* path, TextureFormat format);
		void CreateStack(int width, int height, TextureFormat format,void* data);
		RendererID_t GetID() const { return reinterpret_cast<RendererID_t>(m_ID.Get()); };
		void* GetImGuiID() const { return static_cast<void*>(m_ID.Get()); };
		//TODO: Asset Management
		bool operator==(const Texture2D& texture) const
		{
			return m_ID.Get() == texture.m_ID.Get();
		}
	private:
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_ID;
		unsigned int m_Width, m_Height;
		TextureFormat m_format;
	};
}

