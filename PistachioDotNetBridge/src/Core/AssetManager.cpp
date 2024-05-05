#include "pch.h"
#include "AssetManager.h"

namespace PistachioCS
{
    Asset^ AssetManager::CreateMaterialAsset(System::String^ filename)
    {
        Asset^ asset = gcnew Asset();
        *asset->m_ptr = Pistachio::GetAssetManager()->CreateMaterialAsset((char*)System::Runtime::InteropServices::Marshal::StringToHGlobalAnsi(filename).ToPointer());
        return asset;
    }
    Asset^ AssetManager::CreateTexture2DAsset(System::String^ filename)
    {
        Asset^ asset = gcnew Asset();
        *asset->m_ptr = Pistachio::GetAssetManager()->CreateTexture2DAsset((char*)System::Runtime::InteropServices::Marshal::StringToHGlobalAnsi(filename).ToPointer());
        return asset;
    }
    Asset^ AssetManager::CreateModelAsset(System::String^ filename)
    {
        Asset^ asset = gcnew Asset();
        *asset->m_ptr = Pistachio::GetAssetManager()->CreateModelAsset((char*)System::Runtime::InteropServices::Marshal::StringToHGlobalAnsi(filename).ToPointer());
        return asset;
    }
}
