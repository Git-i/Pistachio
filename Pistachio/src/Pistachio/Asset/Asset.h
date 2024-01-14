#pragma once
#include "Pistachio\Core\UUID.h"
namespace Pistachio
{
	enum class PISTACHIO_API ResourceType
	{
		Invalid,
		Texture,
		Model,
		Material,
		Animation,
		Audio,
	};
	class PISTACHIO_API Asset
	{
	public:
		friend class AssetManager;
		friend class Scene;
		~Asset();
		Asset() = default;
		Asset(const Asset&);
		void operator=(const Asset& other);
		void operator=(Asset&& other) noexcept;
		bool operator==(const Asset& other) const { return m_uuid == other.m_uuid; };
		int ViewRefCount();
		ResourceType GetType() const { return m_type; }
	private:
		Asset(UUID, ResourceType);
	private:
		ResourceType m_type = ResourceType::Invalid;
		UUID m_uuid = (UUID)0;
	};
}
