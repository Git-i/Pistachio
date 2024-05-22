#pragma once
#include "Pistachio/Core/FileSystem.h"
#include "script_result.h"
#include "angelscript.h"
#include <cstdio>
#include <fstream>
#include <string>
namespace Pistachio
{
    namespace Scripting
    {
        class CScriptFile
        {
        public:
            CScriptFile();
            void AddRef() const;
            void Release() const;
            bool IsOpen() const;
            std::string ReadString(asUINT numBytes);
            std::string ReadLine(const std::string& delim);
            asQWORD GetSize() const;
            mutable int refCnt;
            File* m_file;
        };
        Base::ScriptResult OpenFile(const std::string& filename, const std::string& mode, CScriptFile* file);
        CScriptFile* OpenFileEx(const std::string& filename, const std::string& mode, CScriptFile* file);
    }
    
}