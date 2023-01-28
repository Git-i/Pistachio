#pragma once

#include <filesystem>
namespace Pistachio {
	class ContentBrowserPanel {
	public:
		ContentBrowserPanel();
		void OnImGuiRender();
	private:
		std::filesystem::path m_CurrentDirectory;
	};
}
