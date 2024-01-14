#include "pch.h"
#include "AssetManager.h"

namespace PistachioCS
{
    Asset^ AssetManager::CreateMaterialAsset(System::String^ filename)
    {
        return gcnew Asset(
           new Pistachio::Asset(Pistachio::GetAssetManager()->CreateMaterialAsset((char*)System::Runtime::InteropServices::Marshal::StringToHGlobalAnsi(filename).ToPointer()) )
        );
        // TODO: insert return statement here
    }
    Asset^ AssetManager::CreateTexture2DAsset(System::String^ filename)
    {
        return gcnew Asset(
            new Pistachio::Asset(Pistachio::GetAssetManager()->CreateTexture2DAsset((char*)System::Runtime::InteropServices::Marshal::StringToHGlobalAnsi(filename).ToPointer()))
        );
        // TODO: insert return statement here
    }
    Asset^ AssetManager::CreateModelAsset(System::String^ filename)
    {
        return gcnew Asset(
            new Pistachio::Asset(Pistachio::GetAssetManager()->CreateModelAsset((char*)System::Runtime::InteropServices::Marshal::StringToHGlobalAnsi(filename).ToPointer()))
        );
        // TODO: insert return statement here
    }
}
