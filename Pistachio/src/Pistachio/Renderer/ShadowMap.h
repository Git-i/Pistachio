#pragma once
#include "../Core.h"
#include "RendererID_t.h"
namespace Pistachio {
	class ShadowMap
	{
	public:
		ShadowMap(const ShadowMap& other);
		ShadowMap() = default;
		void Create(std::uint32_t size);
		void UpdateSize(std::uint32_t size);
		void Clear();
		void Bind(int slot = 0);
		void BindResource(int slot = 0);
		std::uint32_t GetSize() const { return m_size; }
	private:
		std::uint32_t m_size = 0;
		PlatformRendererID_t m_DSV = nullptr;
		PlatformRendererID_t m_SRV = nullptr;
	};
}
