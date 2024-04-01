#pragma once
#include "RefCountedObject.h"
#include "Pistachio/Renderer/Texture.h"
#include "Pistachio/Renderer/Model.h"
#include "Pistachio/Renderer/ShaderAsset.h"
#include "Asset.h"
namespace Pistachio
{
	class Material;
	class PISTACHIO_API AssetManager
	{
	public:
		Asset CreateMaterialAsset(const std::string& filename);
		Asset CreateTexture2DAsset(const std::string& filename);
		Asset CreateModelAsset(const std::string& filename);
		Asset CreateShaderAsset(const std::string& filename);
		std::string GetAssetFileName(const Asset& asset);
		void ReportLiveObjects();
		Material* GetMaterialResource(Asset& a);
		Texture2D* GetTexture2DResource(Asset& a);
		Model* GetModelResource(Asset& a);
		ShaderAsset* GetShaderResource(Asset& a);
		friend class Asset;
	private:
		Asset CreateAsset(const std::string& filename, ResourceType type);
		//intended to only be used by engine developer. it will most likely leak memory otherwise
		Asset FromResource(RefCountedObject* resource, const std::string& str_id, ResourceType type);
	private:
		std::unordered_map<std::string, UUID> pathUUIDMap;
		std::unordered_map<UUID, RefCountedObject*> assetResourceMap;
	};
	PISTACHIO_API AssetManager* GetAssetManager();
}
