#include "ptpch.h"
#include "MaterialEditorPanel.h"
#include "Pistachio\Asset\AssetManager.h"
#include "imgui.h"
#include "imgui_internal.h"
#include "Pistachio\Renderer\Renderer.h"
#include "Pistachio\Renderer\RendererBase.h"
namespace Pistachio
{
	MaterialEditorPanel::MaterialEditorPanel()
		: cam(Matrix4::Identity)
	{
		RenderTextureDesc desc;
		desc.height = 512;
		desc.width = 512;
		desc.miplevels = 1;
		desc.Attachments = {TextureFormat::RGBA8U};
		previewRT.CreateStack(desc);
		sphere = GetAssetManager()->CreateModelAsset("circle.obj");
		TransformData td;
		td.transform = DirectX::XMMatrixIdentity();
		td.normal = DirectX::XMMatrixIdentity();
		cb.Create(&td, sizeof(TransformData));
	}
	void MaterialEditorPanel::OnImGuiRender()
	{
		if (!activated)
			return;
		else
		{
			if (ImGui::Begin("Material Editor", &activated))
			{
				ImRect rect;
				rect.Min = ImGui::GetWindowPos();
				rect.Max = ImVec2(ImGui::GetWindowPos().x + ImGui::GetWindowSize().x, ImGui::GetWindowPos().y + ImGui::GetWindowSize().y);
				if (ImGui::BeginDragDropTargetCustom(rect, 19))
				{
					const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("MATERIAL");
					if (payload)
					{
						const wchar_t* data = (const wchar_t*)payload->Data;
						std::wstring str(data);
						auto path = std::filesystem::path("assets") / data;
						filepath = path.string();
						mat = GetAssetManager()->CreateMaterialAsset(filepath);
					}
					ImGui::EndDragDropTarget();
				}
				if (mat.GetType() == ResourceType::Material)
				{
					auto material = GetAssetManager()->GetMaterialResource(mat);
					auto mesh = GetAssetManager()->GetModelResource(sphere);
					float previewWidth = ImGui::GetWindowWidth();
					float previewHeight = previewWidth * (0.25f);
					cam.GetProjection() = Matrix4::CreatePerspectiveFieldOfView(45.f, 4, 0.1f, 50.f);
					Renderer::BeginScene(&cam, DirectX::XMMatrixTranslation(0,0,-3));
					previewRT.Bind();
					Shader::SetVSBuffer(cb, 1);
					Renderer::Submit(&mesh->meshes[0], Renderer::GetShaderLibrary().Get("PBR-Forward-Shader").get(), material, -1);
					RendererBase::BindMainTarget();
					ImGui::Image(previewRT.GetSRV().ptr, ImVec2(previewWidth, previewHeight));

					if(ImGui::Button("##diffuse", ImVec2(ImGui::GetTextLineHeight(), ImGui::GetTextLineHeight())))
					{
						ImGui::OpenPopup("Diffuse Texture");
					}
					if (ImGui::BeginDragDropTarget())
					{
						const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("TEXTURE");
						if (payload)
						{
							const wchar_t* data = (const wchar_t*)payload->Data;
							std::wstring str(data);
							auto path = std::filesystem::path("assets") / data;
							material->diffuseTex = GetAssetManager()->CreateTexture2DAsset(path.string());
							material->diffuseTexName = std::string(str.begin(), str.end());
							material->bDirty = true;
						}
						ImGui::EndDragDropTarget();
					}
					ImGui::SameLine();
					if (ImGui::ColorEdit4("Diffuse", (float*)&material->diffuseColor), ImGuiColorEditFlags_PickerHueBar) material->Update();

					if (ImGui::Button("##metallic", ImVec2(ImGui::GetTextLineHeight(), ImGui::GetTextLineHeight())))
					{
						ImGui::OpenPopup("Metallic Texture");
					}
					if (ImGui::BeginDragDropTarget())
					{
						const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("TEXTURE");
						if (payload)
						{
							const wchar_t* data = (const wchar_t*)payload->Data;
							std::wstring str(data);
							auto path = std::filesystem::path("assets") / data;
							material->metallicTex = GetAssetManager()->CreateTexture2DAsset(path.string());
							material->metallicTexName = std::string(str.begin(), str.end());
						}
						ImGui::EndDragDropTarget();
					}
					ImGui::SameLine();
					if (ImGui::SliderFloat("Metallic", &material->metallic, 0.f, 1.f)) material->Update();


					if (ImGui::Button("##roughness", ImVec2(ImGui::GetTextLineHeight(), ImGui::GetTextLineHeight())))
					{
						ImGui::OpenPopup("Roughness Texture");
					}
					if (ImGui::BeginDragDropTarget())
					{
						const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("TEXTURE");
						if (payload)
						{
							const wchar_t* data = (const wchar_t*)payload->Data;
							std::wstring str(data);
							auto path = std::filesystem::path("assets") / data;
							material->roughnessTex = GetAssetManager()->CreateTexture2DAsset(path.string());
							material->roughnessTexName = std::string(str.begin(), str.end());
						}
						ImGui::EndDragDropTarget();
					}
					ImGui::SameLine();
					if (ImGui::SliderFloat("Roughness", &material->roughness, 0.f, 1.f)) material->Update();


					if (ImGui::Button("Save To File"))
					{
						MaterialSerializer ms;
						ms.Serialize(filepath, *material);
					}
					if (ImGui::BeginPopup("Roughness Texture"))
					{
						auto roughnessTex = GetAssetManager()->GetTexture2DResource(material->roughnessTex);
						if (roughnessTex)
							ImGui::Image(roughnessTex->GetID().ptr, ImVec2(512, 512));
						ImGui::Text(material->roughnessTexName.c_str());
						if (ImGui::Button("Remove Texture"))
						{
							material->roughnessTex = Asset();
							ImGui::CloseCurrentPopup();
						}
						ImGui::EndPopup();
					}
					if (ImGui::BeginPopup("Metallic Texture"))
					{
						auto metallicTex = GetAssetManager()->GetTexture2DResource(material->metallicTex);
						if (metallicTex)
							ImGui::Image(metallicTex->GetID().ptr, ImVec2(512, 512));
						ImGui::Text(material->metallicTexName.c_str());
						if (ImGui::Button("Remove Texture"))
						{
							material->metallicTex = Asset();
							ImGui::CloseCurrentPopup();
						}
						ImGui::EndPopup();
					}
					if (ImGui::BeginPopup("Diffuse Texture"))
					{
						auto diffuseTex = GetAssetManager()->GetTexture2DResource(material->diffuseTex);
						if (diffuseTex)
							ImGui::Image(diffuseTex->GetID().ptr, ImVec2(512, 512));
						ImGui::Text(material->diffuseTexName.c_str());
						if (ImGui::Button("Remove Texture"))
						{
							material->diffuseTex = Asset();
							ImGui::CloseCurrentPopup();
						}
						ImGui::EndPopup();
					}
				}
				else
				{
					ImGui::Text("Drag A Material To Open");
				}
				ImGui::End();
			}
		}
	}
}
