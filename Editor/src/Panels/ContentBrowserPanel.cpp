#include "ptpch.h"
#include "ContentBrowserPanel.h"
#include "imgui.h"
#include "imgui_internal.h"
#include <sstream>
Pistachio::Ref<Pistachio::Texture2D> Pistachio::ContentBrowserPanel::m_objIcon = std::shared_ptr<Pistachio::Texture2D>();
Pistachio::Ref<Pistachio::Texture2D> Pistachio::ContentBrowserPanel::m_txtIcon = std::shared_ptr<Pistachio::Texture2D>();
Pistachio::Ref<Pistachio::Texture2D> Pistachio::ContentBrowserPanel::m_folderIcon = std::shared_ptr<Pistachio::Texture2D>();
Pistachio::Ref<Pistachio::Texture2D> Pistachio::ContentBrowserPanel::m_leftArrow = std::shared_ptr<Pistachio::Texture2D>();
Pistachio::Ref<Pistachio::Texture2D> Pistachio::ContentBrowserPanel::m_cppIcon = std::shared_ptr<Pistachio::Texture2D>();
namespace Pistachio {
	static std::filesystem::path s_AssetPath = "assets";
	ContentBrowserPanel::ContentBrowserPanel()
		:m_CurrentDirectory(s_AssetPath)
	{
		m_objIcon.reset(Pistachio::Texture2D::Create("resources/textures/icons/obj_icon.png"));
		m_txtIcon.reset(Pistachio::Texture2D::Create("resources/textures/icons/txt_icon.png"));
		m_folderIcon.reset(Pistachio::Texture2D::Create("resources/textures/icons/folder_icon.png"));
		m_leftArrow.reset(Pistachio::Texture2D::Create("resources/textures/icons/arrowLeft.png"));
		m_cppIcon.reset(Pistachio::Texture2D::Create("resources/textures/icons/cpp_icon.png"));
	}
	void ContentBrowserPanel::OnImGuiRender()
	{
		if(activated)
		{
			ImGui::Begin("Content Browser", &activated);
			if (m_CurrentDirectory != s_AssetPath)
			{
				if (ImGui::ImageButton(m_leftArrow->GetID().ptr, ImVec2(32, 32))) {
					m_CurrentDirectory = m_CurrentDirectory.parent_path();
				}
				ImGui::SameLine();
			}
			else
			{
				ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0, 0, 0, 0));
				ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0, 0, 0, 0));
				ImGui::ImageButton(m_leftArrow->GetID().ptr, ImVec2(32, 32), ImVec2(0, 0), ImVec2(1, 1), -1, ImVec4(0, 0, 0, 0), ImVec4(0.5, 0.5, 0.5, 1.0));
				ImGui::SameLine();
				ImGui::PopStyleColor(2);
			}
			float lineheight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.f;
			ImGui::BeginChildFrame(10, ImVec2(ImGui::GetWindowWidth() - 10.f, lineheight));
			ImGui::Text(m_CurrentDirectory.string().c_str());
			ImGui::EndChildFrame();
			ImGui::Columns(ImGui::GetContentRegionAvail().x / 200, 0, false);
			for (auto& directoryEntry : std::filesystem::directory_iterator(m_CurrentDirectory))
			{
				const auto& path = directoryEntry.path();
				std::string pathStr = path.string();
				auto relativePath = std::filesystem::relative(directoryEntry.path(), s_AssetPath);
				std::string relativePathStr = relativePath.filename().string();
				ImGui::PushID(relativePathStr.c_str());
				if (directoryEntry.is_directory()) {
					ImGui::ImageButton(m_folderIcon->GetID().ptr, ImVec2(200, 200));
					if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0)) {
						m_CurrentDirectory /= path.filename();
					}
					ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (ImGui::GetColumnWidth() - ImGui::CalcTextSize(relativePathStr.c_str()).x) / 2);
					ImGui::Text(relativePathStr.c_str());
				}
				else
				{
					if (relativePath.extension().string() == ".obj")
					{
						ImGui::ImageButton(m_objIcon->GetID().ptr, ImVec2(200, 200));
						if (ImGui::BeginDragDropSource())
						{
							const wchar_t* data = relativePath.c_str();
							ImGui::SetDragDropPayload("3D_MODEL", data, (wcslen(data) + 1) * sizeof(wchar_t), ImGuiCond_Once);
							ImGui::SetTooltip(relativePathStr.c_str());
							ImGui::EndDragDropSource();
						}
					}
					else if(relativePath.extension().string() == ".png")
					{
						ImGui::ImageButton(m_objIcon->GetID().ptr, ImVec2(200, 200));
						if (ImGui::BeginDragDropSource())
						{
							const wchar_t* data = relativePath.c_str();
							ImGui::SetDragDropPayload("TEXTURE", data, (wcslen(data) + 1) * sizeof(wchar_t), ImGuiCond_Once);
							ImGui::SetTooltip(relativePathStr.c_str());
							ImGui::EndDragDropSource();
						}
					}
					else if (relativePath.extension().string() == ".tga")
					{
						ImGui::ImageButton(m_objIcon->GetID().ptr, ImVec2(200, 200));
						if (ImGui::BeginDragDropSource())
						{
							const wchar_t* data = relativePath.c_str();
							ImGui::SetDragDropPayload("TEXTURE", data, (wcslen(data) + 1) * sizeof(wchar_t), ImGuiCond_Once);
							ImGui::SetTooltip(relativePathStr.c_str());
							ImGui::EndDragDropSource();
						}
					}
					else if (relativePath.extension().string() == ".mat")
					{
						ImGui::ImageButton(m_objIcon->GetID().ptr, ImVec2(200, 200));
						if (ImGui::BeginDragDropSource())
						{
							const wchar_t* data = relativePath.c_str();
							ImGui::SetDragDropPayload("MATERIAL", data, (wcslen(data) + 1) * sizeof(wchar_t), ImGuiCond_Once);
							ImGui::SetTooltip(relativePathStr.c_str());
							ImGui::EndDragDropSource();
						}
					}
					else if (relativePath.extension().string() == ".cpp")
					{
						ImGui::ImageButton(m_cppIcon->GetID().ptr, ImVec2(200, 200));
						if (ImGui::BeginDragDropSource())
						{
							const wchar_t* data = relativePath.c_str();
							ImGui::SetDragDropPayload("CPP_FILE", data, (wcslen(data) + 1) * sizeof(wchar_t), ImGuiCond_Once);
							ImGui::SetTooltip(relativePathStr.c_str());
							ImGui::EndDragDropSource();
						}
						TextEditor::LanguageDefinition cpp = TextEditor::LanguageDefinition::CPlusPlus();
						editor.SetLanguageDefinition(cpp);
						editor.SetPalette(TextEditor::GetDarkPalette());
						if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0)) {
							{
								std::ifstream t(path.c_str());
								if (t.good())
								{
									std::string str((std::istreambuf_iterator<char>(t)), std::istreambuf_iterator<char>());
									editor.SetText(str);
									if (editor.GetTotalLines() > 1000)
										editor.SetColorizerEnable(false);
									else
										editor.SetColorizerEnable(true);
								}
							}
							opened = true;
							m_CurrentFile = path;
						}
						if (opened && m_CurrentFile == path)
						{
							ImGui::Begin("Text Editor", &opened, (editor.CanUndo() ? ImGuiWindowFlags_UnsavedDocument : 0) | ImGuiWindowFlags_HorizontalScrollbar | ImGuiWindowFlags_MenuBar);

							if (ImGui::BeginMenuBar())
							{
								if (ImGui::BeginMenu("File"))
								{
									if (ImGui::MenuItem("Save", "Ctrl+S"))
									{
										auto textToSave = editor.GetText();
										std::ofstream t(path.c_str());
										t.write(textToSave.c_str(), textToSave.size());
										t.close();
										/// save text....
									}
									if (ImGui::MenuItem("Close", "Ctrl+W"))
									{
										opened = false;
									}
									ImGui::EndMenu();
								}
								if (ImGui::BeginMenu("Edit"))
								{
									bool ro = editor.IsReadOnly();
									if (ImGui::MenuItem("Read-only mode", nullptr, &ro))
										editor.SetReadOnly(ro);
									ImGui::Separator();

									if (ImGui::MenuItem("Undo", "ALT-Backspace", nullptr, !ro && editor.CanUndo()))
										editor.Undo();
									if (ImGui::MenuItem("Redo", "Ctrl-Y", nullptr, !ro && editor.CanRedo()))
										editor.Redo();

									ImGui::Separator();
									if (ImGui::MenuItem("Copy", "Ctrl-C", nullptr, editor.HasSelection()))
										editor.Copy();
									if (ImGui::MenuItem("Cut", "Ctrl-X", nullptr, !ro && editor.HasSelection()))
										editor.Cut();
									if (ImGui::MenuItem("Delete", "Del", nullptr, !ro && editor.HasSelection()))
										editor.Delete();
									if (ImGui::MenuItem("Paste", "Ctrl-V", nullptr, !ro && ImGui::GetClipboardText() != nullptr))
										editor.Paste();

									ImGui::Separator();

									if (ImGui::MenuItem("Select all", nullptr, nullptr))
										editor.SetSelection(TextEditor::Coordinates(), TextEditor::Coordinates(editor.GetTotalLines(), 0));

									ImGui::EndMenu();
								}
								ImGui::EndMenuBar();
							}
							auto cpos = editor.GetCursorPosition();
							ImGui::Text("%6d/%-6d %6d lines | %s | %s | %s | %s", cpos.mLine + 1, cpos.mColumn + 1, editor.GetTotalLines(),
								editor.IsOverwrite() ? "Ovr" : "Ins",
								editor.CanUndo() ? "*" : " ",
								editor.GetLanguageDefinition().mName.c_str(),
								pathStr.c_str());
							editor.Render((relativePathStr + "##editor").c_str());
							ImGui::End();
						}
					}
					else if (relativePath.extension().string() == ".txt")
					{
						ImGui::ImageButton(m_txtIcon->GetID().ptr, ImVec2(200, 200));
						if (ImGui::BeginDragDropSource())
						{
							const wchar_t* data = relativePath.c_str();
							ImGui::SetDragDropPayload("TXT_FILE", data, (wcslen(data) + 1) * sizeof(wchar_t), ImGuiCond_Once);
							ImGui::SetTooltip(relativePathStr.c_str());
							ImGui::EndDragDropSource();
						}
						TextEditor::LanguageDefinition txt;
						txt.mName = "Plain Text";
						editor.SetLanguageDefinition(txt);
						if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0)) {
							{
								std::ifstream t(path.c_str());
								if (t.good())
								{
									std::string str((std::istreambuf_iterator<char>(t)), std::istreambuf_iterator<char>());
									editor.SetText(str);
								}
							}
							opened = true;
							m_CurrentFile = path;
						}
						if (opened && m_CurrentFile == path)
						{
							ImGui::Begin("Text Editor", &opened, (editor.CanUndo() ? ImGuiWindowFlags_UnsavedDocument : 0) | ImGuiWindowFlags_HorizontalScrollbar | ImGuiWindowFlags_MenuBar);
							editor.SetColorizerEnable(false);
							if (ImGui::BeginMenuBar())
							{
								if (ImGui::BeginMenu("File"))
								{
									if (ImGui::MenuItem("Save", "Ctrl+S"))
									{
										auto textToSave = editor.GetText();
										std::ofstream t(path.c_str());
										t.write(textToSave.c_str(), textToSave.size());
										t.close();
										/// save text....
									}
									if (ImGui::MenuItem("Close", "Ctrl+W"))
									{
										opened = false;
									}
									ImGui::EndMenu();
								}
								if (ImGui::BeginMenu("Edit"))
								{
									bool ro = editor.IsReadOnly();
									if (ImGui::MenuItem("Read-only mode", nullptr, &ro))
										editor.SetReadOnly(ro);
									ImGui::Separator();

									if (ImGui::MenuItem("Undo", "ALT-Backspace", nullptr, !ro && editor.CanUndo()))
										editor.Undo();
									if (ImGui::MenuItem("Redo", "Ctrl-Y", nullptr, !ro && editor.CanRedo()))
										editor.Redo();

									ImGui::Separator();
									TextEditor::GetDarkPalette();
									if (ImGui::MenuItem("Copy", "Ctrl-C", nullptr, editor.HasSelection()))
										editor.Copy();
									if (ImGui::MenuItem("Cut", "Ctrl-X", nullptr, !ro && editor.HasSelection()))
										editor.Cut();
									if (ImGui::MenuItem("Delete", "Del", nullptr, !ro && editor.HasSelection()))
										editor.Delete();
									if (ImGui::MenuItem("Paste", "Ctrl-V", nullptr, !ro && ImGui::GetClipboardText() != nullptr))
										editor.Paste();

									ImGui::Separator();

									if (ImGui::MenuItem("Select all", nullptr, nullptr))
										editor.SetSelection(TextEditor::Coordinates(), TextEditor::Coordinates(editor.GetTotalLines(), 0));

									ImGui::EndMenu();
								}
								ImGui::EndMenuBar();
							}
							auto cpos = editor.GetCursorPosition();
							ImGui::Text("%6d/%-6d %6d lines | %s | %s | %s | %s", cpos.mLine + 1, cpos.mColumn + 1, editor.GetTotalLines(),
								editor.IsOverwrite() ? "Ovr" : "Ins",
								editor.CanUndo() ? "*" : " ",
								editor.GetLanguageDefinition().mName.c_str(),
								pathStr.c_str());
							editor.Render((relativePathStr + "##editor").c_str());
							ImGui::End();
						}
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
}