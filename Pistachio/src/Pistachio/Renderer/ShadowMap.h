#pragma once
#include "../Core.h"
#include "RendererID_t.h"
#include "Pistachio\Allocators\AtlasAllocator.h"
namespace Pistachio {
	class PISTACHIO_API ShadowMap
	{
	public:
		ShadowMap(const ShadowMap& other);
		ShadowMap() = default;
		void Create(std::uint32_t size);
		void UpdateSize(std::uint32_t size);
		void Clear(const hRegion& region, float cval = 1.f);
		void Bind(int slot = 0);
		void BindResource(int slot = 0);
		RHI::TextureView* GetView() { return view; };
		RHI::Texture* GetID() { return ID; }
		std::uint32_t GetSize() const { return m_size; }
	private:
		friend class Scene;
		std::uint32_t m_size = 0;
		RHI::TextureView* view;
		RHI::Texture* ID;
		PlatformRendererID_t m_DSV = nullptr;
		PlatformRendererID_t m_SRV = nullptr;
	};
}
