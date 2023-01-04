#pragma once
#include "Texture.h"
namespace Pistachio {
	enum class TextureFormat {
		RGBA16F, RGBA32F, RGBA8U
	};
	class RenderTexture : public Texture {

	public:
		~RenderTexture();
		void Bind(int slot = 0) const override;
		void BindResource(int slot = 0) const;
		void CreateStack(int width, int height, int miplevels = 1, TextureFormat format = TextureFormat::RGBA8U);
		void Clear(float* clearcolor);
		static RenderTexture* Create(int width, int height, int miplevels = 1);
		inline ID3D11ShaderResourceView* GetSRV() { return m_shaderResourceView; };
		inline ID3D11Texture2D* GetRenderTexture() { return m_renderTargetTexture; };
		inline ID3D11RenderTargetView* GetRTV() { return m_renderTargetView; };
		inline ID3D11DepthStencilView* GetDSV() { return m_pDSV; };
		inline unsigned int GetWidth() const override { return m_width; }
		inline unsigned int GetHeight() const override { return m_height; }
	private:
		ID3D11ShaderResourceView* m_shaderResourceView;
		ID3D11Texture2D* m_renderTargetTexture;
		ID3D11RenderTargetView* m_renderTargetView;
		ID3D11DepthStencilView* m_pDSV;
		int m_width, m_height;
	};
	class RenderCubeMap : public Texture{	
	public:
		~RenderCubeMap();
		void Bind(int slot = 0) const override;
		void BindResource(int slot = 0) const;

		void CreateStack(int width, int height, int miplevels = 1);
		void Clear(float* clearcolor, int slot);
		static RenderCubeMap* Create(int width, int height, int miplevels = 1);
		inline ID3D11ShaderResourceView* GetSRV() { return m_shaderResourceView; };
		inline ID3D11Texture2D* GetRenderTexture() { return m_renderTargetTexture; };
		inline unsigned int GetWidth() const override { return m_width; }
		inline unsigned int GetHeight() const override { return m_height; }
	private:
		ID3D11ShaderResourceView* m_shaderResourceView;
		ID3D11Texture2D* m_renderTargetTexture;
		ID3D11RenderTargetView* m_renderTargetView[6];
		ID3D11DepthStencilView* m_pDSV;
		int m_width, m_height;
	};
}
