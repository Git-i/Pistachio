#pragma once
#include "../Core.h"
namespace Pistachio {
	class Texture
	{
	public:
		virtual unsigned int GetHeight() const = 0;
		virtual unsigned int GetWidth() const = 0;
		virtual void Bind(int slot = 0) const = 0;
	};
	class Texture2D : public Texture
	{
	public:
		unsigned int GetHeight() const override;
		unsigned int GetWidth() const override;
		Texture2D() :m_Width(0), m_Height(0){};
		~Texture2D();
		void Bind(int slot = 0) const override;
		static Texture2D* Create(const char* path);
		static Texture2D CreateStack(const char* path);
		unsigned int m_Width, m_Height;
	private:
		#ifdef PISTACHIO_RENDER_API_DX11
			ID3D11ShaderResourceView* pTextureView = NULL;
		#endif // PISTACHIO_RENDER_API_DX11
	};
	class Texture3D : public Texture
	{
	public:
		unsigned int GetHeight() const override;
		unsigned int GetWidth() const override;
		Texture3D() :m_Width(0), m_Height(0) {};
		void Bind(int slot = 0) const override;
		static Texture3D* Create(const char* path);
		void CreateStack(const char* path);
		unsigned int m_Width, m_Height;
	private:
	#ifdef PISTACHIO_RENDER_API_DX11
		ID3D11ShaderResourceView* pTextureView = NULL;
	#endif // PISTACHIO_RENDER_API_DX11
	};
}

