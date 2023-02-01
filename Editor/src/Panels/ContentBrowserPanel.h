#pragma once
#include "Pistachio/Renderer/Texture.h"
#include <filesystem>
namespace Pistachio {
	class ContentBrowserPanel {
	public:
		ContentBrowserPanel();
		void OnImGuiRender();
	private:
		static Ref<Texture2D> m_objIcon;
		static Ref<Texture2D> m_txtIcon;
		static Ref<Texture2D> m_leftArrow;
		static Ref<Texture2D> m_folderIcon;
		std::filesystem::path m_CurrentDirectory;
	};
}
