#pragma once
#include "../Core.h"
#include "RendererID_t.h"
#include "../Utils/RendererUtils.h"
#include "../Asset/RefCountedObject.h"
#include "FormatsAndTypes.h"
#include "../Core/TextureView.h"
#include "RHIPtr.h"
namespace Pistachio {
	class PISTACHIO_API Texture : public RefCountedObject
	{
	protected:
		Texture() = default;
		friend class Renderer;
		friend class RenderGraph;
		RHIPtr<RHI::Texture> m_ID;
	public:
		virtual RHI::Format GetFormat() const = 0;
		virtual uint32_t GetHeight() const = 0;
		virtual uint32_t GetWidth() const = 0;
	};
	class PISTACHIO_API Texture2D : public Texture
	{
	public:
		RHI::Format GetFormat() const override;
		uint32_t GetHeight() const override;
		uint32_t GetWidth() const override;
		Texture2D() : m_Width(0), m_Height(0), m_format(RHI::Format::UNKNOWN){};
		void Bind(int slot = 0) const;
		static Texture2D* Create(const char* path, RHI::Format format = RHI::Format::R8G8B8A8_UNORM, TextureFlags flags = TextureFlags::NONE);
		static Texture2D* Create(uint32_t width, uint32_t height, RHI::Format format,void* data, TextureFlags flags = TextureFlags::NONE);
		void CreateStack(const char* path, RHI::Format format, TextureFlags flags = TextureFlags::NONE);
		void CreateStack(uint32_t width, uint32_t height, RHI::Format format,void* data, TextureFlags flags = TextureFlags::NONE);
		void CopyIntoRegion(Texture2D& source, unsigned int location_x, unsigned int location_y, unsigned int src_left, unsigned int src_right, unsigned int src_up, unsigned int src_down, unsigned int mipSlice = 0, unsigned int arraySlice = 0);
		void CopyInto(Texture2D& source);
		void CopyToCPUBuffer(void* buffer);
		RHI::TextureView* GetView()const { return m_view.Get(); }
		//TODO: Asset Management
		bool operator==(const Texture2D& texture) const;
		friend class RenderTexture;
		friend class RenderCubeMap;
	private:
		
		unsigned int m_Width, m_Height, m_MipLevels;
		RHI::Format m_format;
		RHIPtr<RHI::TextureView> m_view;
	private:
		void CreateTexture(void* data, TextureFlags flags);
	};
}

