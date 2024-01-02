#pragma once
#include "Pistachio\Core\Math.h"
namespace Pistachio
{
	struct Region
	{
		Vector2 offset;
		Vector2 size;
	};
	class AtlasAllocator
	{
	public:
		AtlasAllocator();
		AtlasAllocator(std::uint32_t width, std::uint32_t height);
		AtlasAllocator(const AtlasAllocator& other);
		Region Allocate(std::uint32_t width, std::uint32_t height);
		void Deallocate(Region region);
	private:
		Vector2 m_size;
	};
}
