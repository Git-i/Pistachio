#pragma once
#include "Pistachio/Renderer/Texture.h"
#include <filesystem>
#include "../../text-editor/TextEditor.h"
namespace Pistachio {
	class ContentBrowserPanel {
	public:
		ContentBrowserPanel();
		void OnImGuiRender();
		bool activated = true;
	private:
		static Ref<Texture2D> m_objIcon;
		static Ref<Texture2D> m_txtIcon;
		static Ref<Texture2D> m_leftArrow;
		static Ref<Texture2D> m_folderIcon;
		static Ref<Texture2D> m_cppIcon;
		bool opened = false;
		TextEditor editor;
		std::filesystem::path m_CurrentDirectory;
		std::filesystem::path m_CurrentFile;
	};
}
