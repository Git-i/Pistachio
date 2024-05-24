#include "ScriptAPICommon.h"
namespace Pistachio
{
    namespace Scripting
    {
        namespace Asset
        {
            void Initialize()
            {
                engine->SetDefaultNamespace("Pistachio::Asset");
                
            }
        }
    }
}