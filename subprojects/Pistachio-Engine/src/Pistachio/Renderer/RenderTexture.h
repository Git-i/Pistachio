#pragma once
#include "Pistachio/Core/Log.h"
#include "Texture.h"
#include "RendererBase.h"
#include "RendererID_t.h"
namespace Pistachio {
	/*
	 * A Render texture is an RT that can propably be later sampled, so the only difference between
	 * this and a normal texture is the color attachment usage.
	 * 
	 * This would be not be a full set of textures as in the old api, just one texture.
	 * The set mechanic would be moved to the render Graph
	 * 
	 * Render textures and cubeMaps are by nature transient resources,
	 * and as such, should be handled by the render graph.
	 * that being said functions are still provided to transition these resources on a given command list
	 * 
	 * 
	*/
	class PISTACHIO_API RenderTexture : public Texture 
	{

	public:
		static RenderTexture* Create(uint32_t width, uint32_t height, uint32_t mipLevels, RHI::Format format PT_DEBUG_REGION(,const char* name));
		void CreateStack(uint32_t width, uint32_t height, uint32_t mipLevels, RHI::Format format PT_DEBUG_REGION(,const char* name));
		void SwitchToRenderTargetMode(RHI::GraphicsCommandList* list);
		void SwitchToShaderUsageMode( RHI::GraphicsCommandList* list);
		RHI::TextureView* GetView()  const{ return m_view; }
		RHI::Format GetFormat() const override;
		RHI::Ptr<RHI::Texture> GetID() const { return m_ID; }
		uint32_t GetWidth() const override;
		uint32_t GetHeight() const override ;

	private:
		friend class RenderGraph;
		RTVHandle RTView;
		RHI::TextureView* m_view;
		RHI::Format m_format;
		uint32_t m_width, m_height, m_mipLevels;
	};
	class PISTACHIO_API DepthTexture : public Texture
	{
	public:
		static DepthTexture* Create(uint32_t width, uint32_t height, uint32_t mipLevels, RHI::Format format PT_DEBUG_REGION(,const char* name));
		void CreateStack(uint32_t width, uint32_t height, uint32_t mipLevels, RHI::Format format PT_DEBUG_REGION(,const char* name));
		RHI::TextureView* GetView() { return m_view; }
		RHI::Format GetFormat() const override;
		uint32_t GetWidth() const override;
		uint32_t GetHeight() const override;
	private:
		friend class RenderGraph;
		friend class Scene;
		DSVHandle DSView;
		RHI::TextureView* m_view;
		RHI::Format m_format;
		uint32_t m_width, m_height, m_mipLevels;
	};
	class PISTACHIO_API RenderCubeMap : public Texture
	{
	public:
		static RenderCubeMap* Create(uint32_t width, uint32_t height, uint32_t mipLevels, RHI::Format format PT_DEBUG_REGION(,const char* name), RHI::TextureUsage extraUsage = RHI::TextureUsage::None);
		void CreateStack(uint32_t width, uint32_t height, uint32_t mipLevels, RHI::Format format PT_DEBUG_REGION(,const char* name), RHI::TextureUsage extraUsage=RHI::TextureUsage::None);
		RHI::Ptr<RHI::TextureView> GetView() { return m_view; }
		void SwitchToRenderTargetMode(RHI::GraphicsCommandList* list=nullptr);
		void SwitchToShaderUsageMode( RHI::GraphicsCommandList* list=nullptr);
		inline unsigned int GetWidth() const override { return m_width; }
		inline unsigned int GetHeight() const override { return m_height; }
		inline RHI::Format GetFormat() const override { return m_format; }
	private:
		friend class RenderGraph;
		friend class Renderer;
		RTVHandle RTViews[6];
		RHI::Format m_format;
		RHI::Ptr<RHI::TextureView> m_view;
		int m_width, m_height, m_mipLevels;
	};
}
