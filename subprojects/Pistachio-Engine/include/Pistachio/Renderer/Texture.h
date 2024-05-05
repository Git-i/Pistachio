#pragma once
#include "../Core.h"
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
		Texture2D() :m_Width(0), m_Height(0){};
		~Texture2D();
		void Bind(int slot = 0) const;
		static Texture2D* Create(const char* path);
		static Texture2D* Create(int width, int height, TextureFormat format,void* data);
		void CreateStack(const char* path);
		void CreateStack(int width, int height, TextureFormat format,void* data);
		ID3D11ShaderResourceView* GetSRV() const { return pTextureView; };
		//TODO: Asset Management
		bool operator==(const Texture2D& texture) const
		{
			return pTextureView == texture.pTextureView;
		}
	private:
		#ifdef PISTACHIO_RENDER_API_DX11
			ID3D11ShaderResourceView* pTextureView = NULL;
		#endif // PISTACHIO_RENDER_API_DX11
			unsigned int m_Width, m_Height;
			TextureFormat m_format;
	};
	class FloatTexture2D : public Texture
	{
	public:
		unsigned int GetHeight() const override;
		unsigned int GetWidth() const override;
		FloatTexture2D() :m_Width(0), m_Height(0) {};
		~FloatTexture2D();
		void Bind(int slot = 0) const;
		static FloatTexture2D* Create(const char* path);
		void CreateStack(const char* path);
		unsigned int m_Width, m_Height;
	private:
		mutable int slot;
	#ifdef PISTACHIO_RENDER_API_DX11
		ID3D11ShaderResourceView* pTextureView = NULL;
	#endif // PISTACHIO_RENDER_API_DX11
	};
}

