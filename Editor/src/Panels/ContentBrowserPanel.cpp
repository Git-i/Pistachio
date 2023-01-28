#include "ptpch.h"
#include "ContentBrowserPanel.h"
#include "imgui.h"
namespace Pistachio {
	static std::filesystem::path s_AssetPath = "assets";
	ContentBrowserPanel::ContentBrowserPanel()
		:m_CurrentDirectory(s_AssetPath)
	{
	}
	void ContentBrowserPanel::OnImGuiRender()
	{
		ImGui::Begin("Content Browser");
		if (m_CurrentDirectory != s_AssetPath)
		{
			if (ImGui::Button("<-"))
				m_CurrentDirectory = m_CurrentDirectory.parent_path();
		}
		for (auto& directoryEntry : std::filesystem::directory_iterator(m_CurrentDirectory))
		{
			const auto& path = directoryEntry.path();
			std::string pathStr = path.string();
			auto relativePath = std::filesystem::relative(directoryEntry.path(), s_AssetPath);
			std::string relativePathStr = relativePath.filename().string();
			if (directoryEntry.is_directory()) {
				
				ImGui::Button(relativePathStr.c_str());
				if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0)) {
					m_CurrentDirectory /= path.filename();
				}
			}
			else
			{
				ImGui::Button(relativePathStr.c_str());
				if (ImGui::BeginDragDropSource())
				{
					const wchar_t* data = relativePath.c_str();
					ImGui::SetDragDropPayload("CONTENT_BROWSER_ITEM", data, (wcslen(data)+1) * sizeof(wchar_t), ImGuiCond_Once);
					ImGui::SetTooltip(relativePathStr.c_str());
					ImGui::EndDragDropSource();
				}
				if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0)) {
				}
			}
		}
		ImGui::End();
	}
}