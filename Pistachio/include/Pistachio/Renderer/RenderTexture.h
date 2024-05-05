#pragma once
#include "Texture.h"
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
		void BindResource(int slot = 0, int count = 1) const;
		void CreateStack(const RenderTextureDesc& desc);
		void Clear(float* clearcolor, int slot = 0);
		void Resize(int width, int height);
		static RenderTexture* Create(const RenderTextureDesc& desc);
		inline ID3D11ShaderResourceView* GetSRV(int slot = 0) const{ return m_shaderResourceView[slot]; };
		inline void GetRenderTexture(ID3D11Resource** pTexture, int slot = 0)const { m_shaderResourceView[slot]->GetResource(pTexture); };
		inline ID3D11RenderTargetView* GetRTV(int slot = 0) { return m_renderTargetView[slot]; };
		inline ID3D11DepthStencilView* GetDSV() { return m_pDSV; };
		inline void GetDepthTexture(ID3D11Resource** pTexture) { return m_pDSV->GetResource(pTexture); };
		inline unsigned int GetWidth() const override { return m_width; }
		inline unsigned int GetHeight() const override { return m_height; }
	private:
		std::vector<ID3D11ShaderResourceView*> m_shaderResourceView;
		std::vector<ID3D11RenderTargetView*> m_renderTargetView;
		ID3D11DepthStencilView* m_pDSV;
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
