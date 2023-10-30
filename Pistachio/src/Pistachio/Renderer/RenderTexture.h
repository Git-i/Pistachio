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
		void Bind(int slot = 0, int count = 1) const;
		void BindResource(int slot = 0, int count = 1, int index = 0) const;
		void CreateStack(const RenderTextureDesc& desc);
		void Clear(float* clearcolor, int slot = 0);
		void ClearDepth();
		void ClearAll(float* clearcolor);
		static RenderTexture* Create(const RenderTextureDesc& desc);
		RendererID_t GetSRV(int slot = 0) const; 
		Texture2D GetRenderTexture(int slot = 0)const;
		RendererID_t GetRTV(int slot = 0); 
		RendererID_t GetDSV();
		Texture2D GetDepthTexture() const;
		inline unsigned int GetWidth() const override { return m_width; }
		inline unsigned int GetHeight() const override { return m_height; }
	private:
		std::vector<PlatformRendererID_t> m_shaderResourceView;
		std::vector<PlatformRendererID_t> m_renderTargetView;
		PlatformRendererID_t m_pDSV = { 0 };
		int m_width, m_height, m_miplevels;
	};
	class RenderCubeMap : public Texture{	
	public:
		~RenderCubeMap() = default;
		void Bind(int slot = 0) const;
		void BindResource(int slot = 0) const;
		void CreateStack(int width, int height, int miplevels = 1);
		void Clear(float* clearcolor, int slot);
		static RenderCubeMap* Create(int width, int height, int miplevels = 1);
		RendererID_t GetID();
		Texture2D GetResource();
		RendererID_t Get_RTID(int slot = 0);
		inline unsigned int GetWidth() const override { return m_width; }
		inline unsigned int GetHeight() const override { return m_height; }
	private:
		PlatformRendererID_t m_shaderResourceView;
		PlatformRendererID_t m_renderTargetView[6];
		PlatformRendererID_t m_pDSV;
		int m_width, m_height, m_mipLevels;
	};
}
