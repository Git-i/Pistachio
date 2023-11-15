#pragma once
#include "RefCountedObject.h"
#include "Pistachio/Renderer/Texture.h"
#include "Pistachio/Renderer/Model.h"
#include "Pistachio/Renderer/Material.h"
#include "Asset.h"
namespace Pistachio
{
	class AssetManager
	{
	public:
		Asset CreateMaterialAsset(const std::string& filename);
		Asset CreateTexture2DAsset(const std::string& filename);
		Asset CreateModelAsset(const std::string& filename);
		std::string GetAssetFileName(Asset& asset);
		void ReportLiveObjects();
		Material* GetMaterialResource(Asset& a);
		Texture2D* GetTexture2DResource(Asset& a);
		Model* GetModelResource(Asset& a);
		friend class Asset;
	private:
		Asset CreateAsset(const std::string& filename, ResourceType type);
	private:
		std::unordered_map<std::string, UUID> pathUUIDMap;
		std::unordered_map<UUID, RefCountedObject*> assetResourceMap;
	};
 AssetManager* GetAssetManager();
}
