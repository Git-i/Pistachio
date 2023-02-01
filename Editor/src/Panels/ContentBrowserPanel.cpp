#include "ptpch.h"
#include "ContentBrowserPanel.h"
#include "imgui.h"
#include "imgui_internal.h"
Pistachio::Ref<Pistachio::Texture2D> Pistachio::ContentBrowserPanel::m_objIcon = std::shared_ptr<Pistachio::Texture2D>();
Pistachio::Ref<Pistachio::Texture2D> Pistachio::ContentBrowserPanel::m_txtIcon = std::shared_ptr<Pistachio::Texture2D>();
Pistachio::Ref<Pistachio::Texture2D> Pistachio::ContentBrowserPanel::m_folderIcon = std::shared_ptr<Pistachio::Texture2D>();
Pistachio::Ref<Pistachio::Texture2D> Pistachio::ContentBrowserPanel::m_leftArrow = std::shared_ptr<Pistachio::Texture2D>();
namespace Pistachio {
	static std::filesystem::path s_AssetPath = "assets";
	ContentBrowserPanel::ContentBrowserPanel()
		:m_CurrentDirectory(s_AssetPath)
	{
		m_objIcon.reset(Pistachio::Texture2D::Create("resources/textures/icons/obj_icon.png"));
		m_txtIcon.reset(Pistachio::Texture2D::Create("resources/textures/icons/txt_icon.png"));
		m_folderIcon.reset(Pistachio::Texture2D::Create("resources/textures/icons/folder_icon.png"));
		m_leftArrow.reset(Pistachio::Texture2D::Create("resources/textures/icons/arrowLeft.png"));
	}
	void ContentBrowserPanel::OnImGuiRender()
	{
		ImGui::Begin("Content Browser");
		if (m_CurrentDirectory != s_AssetPath)
		{
			if (ImGui::ImageButton(m_leftArrow->GetSRV(), ImVec2(32, 32))) {
				m_CurrentDirectory = m_CurrentDirectory.parent_path();
			}
				ImGui::SameLine();
		}
		else
		{
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0, 0, 0, 0));
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0, 0, 0, 0));
			ImGui::ImageButton(m_leftArrow->GetSRV(), ImVec2(32, 32), ImVec2(0,0), ImVec2(1,1), -1, ImVec4(0,0,0,0), ImVec4(0.5,0.5,0.5,1.0));
			ImGui::SameLine();
			ImGui::PopStyleColor(2);
		}
		float lineheight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.f;
		ImGui::BeginChildFrame(10, ImVec2(ImGui::GetWindowWidth()-10.f, lineheight));
		ImGui::Text(m_CurrentDirectory.string().c_str());
		ImGui::EndChildFrame();
		ImGui::Columns(ImGui::GetColumnWidth() / 200, 0, false);
		for (auto& directoryEntry : std::filesystem::directory_iterator(m_CurrentDirectory))
		{
			const auto& path = directoryEntry.path();
			std::string pathStr = path.string();
			auto relativePath = std::filesystem::relative(directoryEntry.path(), s_AssetPath);
			std::string relativePathStr = relativePath.filename().string();
			ImGui::PushID(relativePathStr.c_str());
			if (directoryEntry.is_directory()) {
				
				ImGui::ImageButton(m_folderIcon->GetSRV(), ImVec2(200,200));
				if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0)) {
					m_CurrentDirectory /= path.filename();
				}
				ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (ImGui::GetColumnWidth() - ImGui::CalcTextSize(relativePathStr.c_str()).x)/2);
				ImGui::Text(relativePathStr.c_str());
			}
			else
			{
				if (relativePath.extension().string() == ".obj")
				{
					ImGui::ImageButton(m_objIcon->GetSRV(), ImVec2(200, 200));
					if (ImGui::BeginDragDropSource())
					{
						const wchar_t* data = relativePath.c_str();
						ImGui::SetDragDropPayload("3D_MODEL", data, (wcslen(data)+1) * sizeof(wchar_t), ImGuiCond_Once);
						ImGui::SetTooltip(relativePathStr.c_str());
						ImGui::EndDragDropSource();
					}
				}
				else if (relativePath.extension().string() == ".txt")
				{
					ImGui::ImageButton(m_txtIcon->GetSRV(), ImVec2(200, 200));
					if (ImGui::BeginDragDropSource())
					{
						const wchar_t* data = relativePath.c_str();
						ImGui::SetDragDropPayload("TXT_FILE", data, (wcslen(data) + 1) * sizeof(wchar_t), ImGuiCond_Once);
						ImGui::SetTooltip(relativePathStr.c_str());
						ImGui::EndDragDropSource();
					}
				}
				if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0)) {
				}
				ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (ImGui::GetColumnWidth() - ImGui::CalcTextSize(relativePathStr.c_str()).x) / 2);
				ImGui::Text(relativePathStr.c_str());
			}
			ImGui::NextColumn();
			ImGui::PopID();
		}
		ImGui::End();
	}
}