#pragma once
#include "Pistachio\Core\UUID.h"
namespace Pistachio
{
	enum class ResourceType
	{
		Invalid,
		Texture,
		Model,
		Material,
		Animation,
		Audio,
	};
	class Asset
	{
	public:
		friend class AssetManager;
		~Asset();
		Asset() = default;
		Asset(const Asset&);
		void operator=(const Asset& other);
		void operator=(Asset&& other);
		bool operator==(const Asset& other) const { return m_uuid == other.m_uuid; };
		int ViewRefCount();
	private:
		Asset(UUID, ResourceType);
	private:
		ResourceType m_type = ResourceType::Invalid;
		UUID m_uuid = (UUID)0;
	};
}
