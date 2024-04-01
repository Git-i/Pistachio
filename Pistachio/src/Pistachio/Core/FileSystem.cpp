#include "ptpch.h"
#include "FileSystem.h"
#include <filesystem>
namespace Pistachio
{
    File FileSystem::OpenFile(const std::string& in)
    {
        //"in" is expected to be in the format <system>://<path>
        //<system> is expected to be on of "builtin" or "asset"
        
        //retrieve system
        size_t system_name_pos = in.find(':', 0);
        if (system_name_pos == std::string::npos) return File{};
        std::string_view fs_name = std::string_view(in.c_str(), system_name_pos);
        if (fs_name.compare("builtin"))
        {
            //for now the builtin filesystem is in the same directory
            File file;
            file.file.open(in.c_str()+system_name_pos+3, std::fstream::in);
            file.size = std::filesystem::file_size(in.c_str() + system_name_pos + 3);
            return file;
        }
        else if (fs_name.compare("assets"))
        {
            //assuming the assets dir is in the project dir
            std::string path = std::string("assets/") + std::string(in.c_str() + system_name_pos + 3);
            File file;
            file.file.open(path);
            file.size = std::filesystem::file_size(std::move(path));
            return file;
        }
        else
        {
            return File{};
        }
    }
}

