#pragma once
#include "Texture.h"
#include "RendererID_t.h"
namespace Pistachio {
	struct RenderTextureAttachmentSpecification {
		RenderTextureAttachmentSpecification() = default;
		RenderTextureAttachmentSpecification(std::initializer_list<TextureFormat> attachments) : Attachments(attachments) {}
		std::vector<TextureFormat> Attachments;
	};
	struct RenderTextureDesc {
		int width; int height; int miplevels = 1; RenderTextureAttachmentSpecification Attachments;
	};
	class RenderTexture : public Texture {

	public:
		~RenderTexture();
		void Shutdown();
		void Bind(int slot = 0, int count = 1) const;
		void BindResource(int slot = 0, int count = 1, int index = 0) const;
		void CreateStack(const RenderTextureDesc& desc);
		void Clear(float* clearcolor, int slot = 0);
		void Resize(int width, int height);
		static RenderTexture* Create(const RenderTextureDesc& desc);
		RendererID_t GetSRV(int slot = 0) const; 
		void GetRenderTexture(RendererID_t* pTexture, int slot = 0)const;
		RendererID_t GetRTV(int slot = 0); 
		RendererID_t GetDSV();
		void GetDepthTexture(RendererID_t* pTexture) const;
		inline unsigned int GetWidth() const override { return m_width; }
		inline unsigned int GetHeight() const override { return m_height; }
	private:
		std::vector<RendererID_t> m_shaderResourceView;
		std::vector<RendererID_t> m_renderTargetView;
		RendererID_t m_pDSV;
		int m_width, m_height, m_miplevels;
	};
	class RenderCubeMap : public Texture{	
	public:
		~RenderCubeMap();
		void Bind(int slot = 0) const;
		void BindResource(int slot = 0) const;
		void ShutDown();
		void CreateStack(int width, int height, int miplevels = 1);
		void Clear(float* clearcolor, int slot);
		static RenderCubeMap* Create(int width, int height, int miplevels = 1);
		inline ID3D11ShaderResourceView* GetSRV() { return m_shaderResourceView; };
		inline ID3D11Texture2D* GetRenderTexture() { return m_renderTargetTexture; };
		inline ID3D11RenderTargetView* GetRTV(int slot = 0) { return m_renderTargetView[slot]; };
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
