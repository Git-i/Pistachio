#pragma once
#define PT_CUSTOM_ENTRY
#include "Pistachio.h"
#include "Asset.h"
#include "ManagedBase.h"

namespace PistachioCS
{
	public ref class AssetManager
	{
	internal:
		static Pistachio::AssetManager* m_ptr;
	public:
		AssetManager() { m_ptr = Pistachio::GetAssetManager(); }
		static Asset^ CreateMaterialAsset(System::String^ filename);
		static Asset^ CreateTexture2DAsset(System::String^ filename);
		static Asset^ CreateModelAsset(System::String^ filename);
	};
}
