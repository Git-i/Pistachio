#include "ptpch.h"
#include "FileSystem.h"
#include <filesystem>
#include <ios>
#include <limits>
namespace Pistachio
{
    File* FileSystem::OpenFile(const std::string& in)
    {
        //"in" is expected to be in the format <system>://<path>
        //<system> is expected to be on of "builtin" or "asset"
        File* returnVal = new File();
        //retrieve system
        size_t system_name_pos = in.find(':', 0);
        if (system_name_pos == std::string::npos)
        {
            delete returnVal;
            return nullptr;
        } 
            
        std::string_view fs_name = std::string_view(in.c_str(), system_name_pos);
        if (fs_name.compare("builtin"))
        {
            //for now the builtin filesystem is in the same directory
            returnVal->file.open(in.c_str()+system_name_pos+3, std::fstream::in);
            returnVal->size = std::filesystem::file_size(in.c_str() + system_name_pos + 3);
            return returnVal;
        }
        else if (fs_name.compare("assets"))
        {
            //assuming the assets dir is in the project dir
            std::string path = std::string("assets/") + std::string(in.c_str() + system_name_pos + 3);
            returnVal->file.open(path);
            returnVal->size = std::filesystem::file_size(std::move(path));
            return returnVal;
        }

        delete returnVal;
        return nullptr;
    }
    bool File::IsOpen()
    {
        return file.is_open();
    }
	size_t File::GetSize()
    {
        auto pos = file.tellg();
        file.seekg(0, std::ios_base::beg);
        file.ignore(std::numeric_limits<std::streamsize>::max());
        std::streamsize len = file.gcount();
        file.clear();
        file.seekg(pos, std::ios_base::beg);
        return len;
    }
	void File::ReadAllToBuf(std::vector<char>& buf)
    {
        size_t size = GetSize();
        buf.resize(size);
        file.read(buf.data(), size);
    }
	void File::ReadAllToBuf(char* buf)
    {
        size_t size = GetSize();
        file.read(buf, size);
    }
	void File::ReadToBuf(std::vector<char>& buf, uint32_t numBytes)
    {
        size_t size = numBytes;
        buf.resize(size);
        file.read(buf.data(), size);
    }
	void File::ReadToBuf(char* buf, uint32_t numBytes)
    {
        size_t size = numBytes;
        file.read(buf, size);
    }
	std::fstream& File::GetFile()
    {
        return file;
    }

}

