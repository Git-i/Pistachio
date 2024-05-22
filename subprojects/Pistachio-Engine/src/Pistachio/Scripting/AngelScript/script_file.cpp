
#include "angelscript.h"
#include "ptpch.h"
#include "script_file.h"

#include "Pistachio/Core/FileSystem.h"
#include <string>
namespace Pistachio 
{
    namespace Scripting
    {
        CScriptFile::CScriptFile()
        {
            refCnt = 1;
        }
        void CScriptFile::AddRef() const
        {
            refCnt++;
        }
        asQWORD CScriptFile::GetSize() const
        {
            return m_file->GetSize();
        }
        std::string CScriptFile::ReadString(asUINT numBytes)
        {
            std::string retVal;
            retVal.resize(numBytes);
            m_file->ReadToBuf(retVal.data(), numBytes);
            return retVal;
        }
        std::string CScriptFile::ReadLine(const std::string& delim)
        {
            auto& stream = m_file->GetFile();
            std::string line;
            std::getline(stream, line, delim[0]);
            return line;
        }

        Base::ScriptResult OpenFile(const std::string& filename, const std::string& mode, CScriptFile* in)
        {
            in->m_file = FileSystem::OpenFile(filename);
            if(!in->m_file)
            {
                return -1;
            }
            return 0;
        }
    }

}