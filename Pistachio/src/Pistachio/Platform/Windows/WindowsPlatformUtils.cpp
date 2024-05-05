#include "ptpch.h"
#include "Pistachio/Utils/PlatformUtils.h"
#include "Pistachio/Core/Application.h"
namespace Pistachio {
	std::string FileDialogs::OpenFile(const char* filter) {
		OPENFILENAMEA ofn = {};
		char szFile[260] = { 0 };
		ofn.lStructSize = sizeof(OPENFILENAMEA);
		ofn.hwndOwner = Application::Get().GetWindow().pd.hwnd;
		ofn.lpstrFile = szFile;
		ofn.nMaxFile = sizeof(szFile);
		ofn.lpstrFilter = filter;
		ofn.nFilterIndex = 1;
		ofn.Flags = OFN_PATHMUSTEXIST | OFN_NOCHANGEDIR | OFN_FILEMUSTEXIST;
		if (GetOpenFileNameA(&ofn) == TRUE)
		{
			return ofn.lpstrFile;
		}
		return std::string();
	}
	std::string FileDialogs::SaveFile(const char* filter) {
		OPENFILENAMEA ofn = {};
		char szFile[260] = { 0 };
		ofn.lStructSize = sizeof(OPENFILENAMEA);
		ofn.hwndOwner = Application::Get().GetWindow().pd.hwnd;
		ofn.lpstrFile = szFile;
		ofn.nMaxFile = sizeof(szFile);
		ofn.lpstrFilter = filter;
		ofn.nFilterIndex = 1;
		ofn.Flags = OFN_PATHMUSTEXIST | OFN_NOCHANGEDIR | OFN_FILEMUSTEXIST;
		if (GetSaveFileNameA(&ofn) == TRUE)
		{
			return ofn.lpstrFile;
		}
		return std::string();
	}
}