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
		if (other.m_uuid == 0) return;
		auto assetMan = GetAssetManager();
		assetMan->assetResourceMap[other.m_uuid]->hold();
		m_type = other.m_type;
		m_uuid = other.m_uuid;
	}
	void Asset::operator=(const Asset& other)
	{
		if (other.m_uuid == 0) return;
		auto assetMan = GetAssetManager();
		auto res = assetMan->assetResourceMap[m_uuid];
		if (m_uuid) {
			if (res->release() == 0)
			{
				if (m_type == ResourceType::Material) {delete ((Material*)res);}
				if (m_type == ResourceType::Texture) { delete ((Texture2D*)res); }
				if (m_type == ResourceType::Model) { delete ((Model*)res);  }
				assetMan->assetResourceMap.erase(m_uuid);
				auto it = std::find_if(assetMan->pathUUIDMap.begin(), assetMan->pathUUIDMap.end(), [this](auto&& p) { return p.second == m_uuid; });
				assetMan->pathUUIDMap.erase(it->first);
			}
		}
		m_type = other.m_type;
		m_uuid = other.m_uuid;
		assetMan->assetResourceMap[m_uuid]->hold();
	}
	void Asset::operator=(Asset&& other)
	{
		if (other.m_uuid == 0) return;
		auto assetMan = GetAssetManager();
		auto res = assetMan->assetResourceMap[m_uuid];
		if (m_uuid) {
			if (res->release() == 0)
			{
				if (m_type == ResourceType::Material) {  delete ((Material*)res); }
				if (m_type == ResourceType::Texture) { delete ((Texture2D*)res);  }
				if (m_type == ResourceType::Model) { delete ((Model*)res);  }
				assetMan->assetResourceMap.erase(m_uuid);
				auto it = std::find_if(assetMan->pathUUIDMap.begin(), assetMan->pathUUIDMap.end(), [this](auto&& p) { return p.second == m_uuid; });
				assetMan->pathUUIDMap.erase(it->first);
			}
		}
		m_type = other.m_type;
		m_uuid = other.m_uuid;
	}
	Asset::Asset(UUID uuid, ResourceType type)
	{
		GetAssetManager()->assetResourceMap[uuid]->hold();
		m_uuid = uuid;
		m_type = type;
	}
	int Asset::ViewRefCount()
	{
		return GetAssetManager()->assetResourceMap[m_uuid]->count();
	}
	Asset::~Asset()
	{
		if (m_uuid == 0) return;
		auto assetMan = GetAssetManager();
		auto res = assetMan->assetResourceMap[m_uuid];
		if (m_uuid) {
			if (res->release() == 0)
			{
				if (m_type == ResourceType::Material) {
					delete ((Material*)res);
				}
				if (m_type == ResourceType::Texture) { delete ((Texture2D*)res);}
				if (m_type == ResourceType::Model) { delete ((Model*)res);}
				assetMan->assetResourceMap.erase(m_uuid);
				auto it = std::find_if(assetMan->pathUUIDMap.begin(), assetMan->pathUUIDMap.end(), [this](auto&& p) { return p.second == m_uuid; });
				assetMan->pathUUIDMap.erase(it->first);
			}
		}
		m_uuid = 0;
		m_type = ResourceType::Invalid;
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
		return CreateAsset(filename, ResourceType::Material);
	}
	Asset AssetManager::CreateTexture2DAsset(const std::string& filename)
	{
		return CreateAsset(filename, ResourceType::Texture);
	}
	Asset AssetManager::CreateModelAsset(const std::string& filename)
	{
		return CreateAsset(filename, ResourceType::Model);
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
	Model* AssetManager::GetModelResource(Asset& a)
	{
		auto it = assetResourceMap.find(a.m_uuid);
		if (it != assetResourceMap.end())
		{
			return (Model*)(assetResourceMap[a.m_uuid]);
		}
		return nullptr;
	}
	Asset AssetManager::CreateAsset(const std::string& filename, ResourceType type)
	{
		auto it = pathUUIDMap.find(filename);
		if (it != pathUUIDMap.end())
		{
			assetResourceMap[it->second]->hold();
			return Asset(it->second, type);
		}
		else
		{
			UUID uuid = UUID();
			RefCountedObject* obj;
			if (type == ResourceType::Texture) obj = Texture2D::Create(filename.c_str());
			else if (type == ResourceType::Material) obj = Material::Create(filename.c_str());
			else if (type == ResourceType::Model) obj = Model::Create(filename.c_str());
			else obj = new RefCountedObject;
			assetResourceMap[uuid] = obj;
			obj->hold();
			Asset asset = Asset(uuid, type);
			pathUUIDMap[filename] = asset.m_uuid;
			return asset;
		}
	}
}