#include "ptpch.h"
#include "AssetManager.h"

namespace Pistachio
{
	AssetManager* GetAssetManager()
	{
		static AssetManager s_AssetMan;
		return &s_AssetMan;
	}
	Asset::Asset(const Asset& other)
	{
		PT_CORE_INFO(" Copy Constructor Called");
		auto assetMan = GetAssetManager();
		assetMan->assetResourceMap[other.m_uuid]->hold();
		m_type = other.m_type;
		m_uuid = other.m_uuid;
		PT_CORE_ERROR(" Copy Constructor Over");
	}
	void Asset::operator=(const Asset& other)
	{
		PT_CORE_INFO("Assignment Called");
		auto assetMan = GetAssetManager();
		auto res = assetMan->assetResourceMap[m_uuid];
		if (m_uuid) {
			if (res->release() == 0)
			{
				if (m_type == ResourceType::Material) {PT_CORE_WARN("RC Material Deleted");delete ((Material*)res);}
				if (m_type == ResourceType::Texture) { delete ((Texture2D*)res); PT_CORE_WARN("RC Texture Deleted"); }
				assetMan->assetResourceMap.erase(m_uuid);
				auto it = std::find_if(assetMan->pathUUIDMap.begin(), assetMan->pathUUIDMap.end(), [this](auto&& p) { return p.second == m_uuid; });
				assetMan->pathUUIDMap.erase(it->first);
			}
		}
		m_type = other.m_type;
		m_uuid = other.m_uuid;
		assetMan->assetResourceMap[m_uuid]->hold();
		PT_CORE_ERROR("Assignment Over");
	}
	void Asset::operator=(Asset&& other)
	{
		PT_CORE_INFO("Move Assignment Called");
		auto assetMan = GetAssetManager();
		auto res = assetMan->assetResourceMap[m_uuid];
		if (m_uuid) {
			if (res->release() == 0)
			{
				if (m_type == ResourceType::Material) { PT_CORE_WARN("RC Material Deleted"); delete ((Material*)res); }
				if (m_type == ResourceType::Texture) { delete ((Texture2D*)res); PT_CORE_WARN("RC Texture Deleted"); }
				assetMan->assetResourceMap.erase(m_uuid);
				auto it = std::find_if(assetMan->pathUUIDMap.begin(), assetMan->pathUUIDMap.end(), [this](auto&& p) { return p.second == m_uuid; });
				assetMan->pathUUIDMap.erase(it->first);
			}
		}
		m_type = other.m_type;
		m_uuid = other.m_uuid;
		PT_CORE_ERROR("Move Assignment Over");
	}
	Asset::Asset(UUID uuid, ResourceType type)
	{
		PT_CORE_INFO("Constructor Called");
		GetAssetManager()->assetResourceMap[uuid]->hold();
		m_uuid = uuid;
		m_type = type;
		PT_CORE_ERROR("Constructor Over");
	}
	int Asset::ViewRefCount()
	{
		return GetAssetManager()->assetResourceMap[m_uuid]->count();
	}
	Asset::~Asset()
	{
		PT_CORE_INFO("Destructor Called");
		auto assetMan = GetAssetManager();
		auto res = assetMan->assetResourceMap[m_uuid];
		if (m_uuid) {
			if (res->release() == 0)
			{
				if (m_type == ResourceType::Material) {
					PT_CORE_WARN("RC Material Deleted");
					delete ((Material*)res);
				}
				if (m_type == ResourceType::Texture) { delete ((Texture2D*)res); PT_CORE_WARN("RC Texture Deleted"); }
				assetMan->assetResourceMap.erase(m_uuid);
				auto it = std::find_if(assetMan->pathUUIDMap.begin(), assetMan->pathUUIDMap.end(), [this](auto&& p) { return p.second == m_uuid; });
				assetMan->pathUUIDMap.erase(it->first);
			}
		}
		m_uuid = 0;
		m_type = ResourceType::Invalid;
		PT_CORE_ERROR("Destructor Over");
	}
	inline int Pistachio::RefCountedObject::hold() const
	{
		m_count_++;
		std::cout << "rc object held, new count is :" << m_count_ << std::endl;
		return m_count_;
	}
	inline int RefCountedObject::release() const
	{
		assert(m_count_ > 0);
		m_count_--;
		std::cout << "rc object released, new count is :" << m_count_ << std::endl;
		return m_count_;
	}
	inline int RefCountedObject::count()
	{
		return m_count_;
	}
	Asset AssetManager::CreateMaterialAsset(const std::string& filename)
	{
		auto it = pathUUIDMap.find(filename);
		if (it != pathUUIDMap.end())
		{
			assetResourceMap[it->second]->hold();
			return Asset(it->second, ResourceType::Material);
		}
		else
		{
			UUID uuid = UUID();
			auto obj = new Material;
			assetResourceMap[uuid] = obj;
			obj->hold();
			Asset asset = Asset(uuid, ResourceType::Material);
			pathUUIDMap[filename] = asset.m_uuid;
			return asset;
		}
	}
	Asset AssetManager::CreateTexture2DAsset(const std::string& filename)
	{
		auto it = pathUUIDMap.find(filename);
		if (it != pathUUIDMap.end())
		{
			assetResourceMap[it->second]->hold();
			return Asset(it->second, ResourceType::Texture);
		}
		else
		{
			UUID uuid = UUID();
			auto obj = Texture2D::Create(filename.c_str());
			assetResourceMap[uuid] = obj;
			obj->hold();
			Asset asset = Asset(uuid, ResourceType::Texture);
			pathUUIDMap[filename] = asset.m_uuid;
			return asset;
		}
	}
	void AssetManager::ReportLiveObjects()
	{
		for (auto [uuid, res] : assetResourceMap)
		{
			PT_CORE_INFO("Live object with id: {0} at memory location {1}", uuid, (void*)res);
		}
	}
	Material* AssetManager::GetMaterialResource(Asset& a) 
	{ 
		auto it = assetResourceMap.find(a.m_uuid);
		if (it != assetResourceMap.end())
		{
			return (Material*)(assetResourceMap[a.m_uuid]);
		}
		return nullptr;
	}
	Texture2D* AssetManager::GetTexture2DResource(Asset& a) 
	{
		auto it = assetResourceMap.find(a.m_uuid);
		if (it != assetResourceMap.end())
		{
			return (Texture2D*)(assetResourceMap[a.m_uuid]);
		}
		return nullptr;
	}
}