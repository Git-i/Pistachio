#include "ptpch.h"
#include "Scene.h"
#include "Components.h"
#include "Pistachio/Renderer/Renderer2D.h"
#include "Pistachio/Renderer/Renderer.h"
#include "Entity.h"
#include "imgui.h"
#include "ImGuizmo.h"
namespace Pistachio {
	Scene::Scene()
	{

	}
	Scene::~Scene()
	{
	}
	Entity Scene::CreateEntity(const std::string& name)
	{
		Entity entity = {m_Registry.create(), this};
		entity.AddComponent<TransformComponent>();
		auto& tag = entity.AddComponent<TagComponent>();
		tag.Tag = name.empty() ? "Entity" : name;
		return entity;
	}
	void Scene::DestroyEntity(Entity entity)
	{
		m_Registry.destroy(entity);
	}
	Entity Scene::GetPrimaryCameraEntity()
	{
		auto view = m_Registry.view<CameraComponent>();
		for (auto entity : view)
		{
			const auto& camera = view.get<CameraComponent>(entity);
			if (camera.Primary)
				return Entity(entity, this);
		}
		return Entity();
	}
	void Scene::OnUpdateEditor(float delta, EditorCamera& camera)
	{
		{
			auto group = m_Registry.view<TransformComponent, LightComponent>();
			for (auto& entity : group)
			{
				Light light;
				auto [tc, lightcomponent] = group.get<TransformComponent, LightComponent>(entity);
				if (lightcomponent.Type == 0) {
					DirectX::XMVECTOR lightTransform = DirectX::XMVector3Rotate(DirectX::XMVectorSet(0.f,1.f,0.f,1.f), DirectX::XMQuaternionRotationRollPitchYawFromVector(tc.Rotation));
					light.positionxtype = { DirectX::XMVectorGetX(lightTransform), DirectX::XMVectorGetY(lightTransform), DirectX::XMVectorGetZ(lightTransform), 0 };
				}
				else if (lightcomponent.Type == 1)
					light.positionxtype = { DirectX::XMVectorGetX(tc.Translation), DirectX::XMVectorGetY(tc.Translation), DirectX::XMVectorGetZ(tc.Translation), 1 };
				light.colorxintensity = { lightcomponent.color.x, lightcomponent.color.y, lightcomponent.color.z, lightcomponent.Intensity };
				Renderer::AddLight(light);
			}
		}
		Renderer::BeginScene(camera);
		{
			auto group = m_Registry.view<TransformComponent, MeshRendererComponent>();
			for (auto& entity : group)
			{
				//TO-DO MATERIALS
				static float color[4] = { 0.42, 0.22, 0.12,1.f };
				float c[4]{
				std::pow(color[0], 2.2),
				std::pow(color[1], 2.2),
				std::pow(color[2], 2.2),
				std::pow(color[3], 2.2),
				};
				static float mrao[3] = {0.f,0.f,1.f};
				ImGui::ColorEdit3("Color", color);
				ImGui::DragFloat3("MRAO", mrao);
				auto [transform, mesh] = group.get<TransformComponent, MeshRendererComponent>(entity);
				const auto& transformMatrix = transform.GetTransform();
				if ((transform.NumNegativeScaleComps % 2))
				{
					RendererBase::SetCullMode(CullMode::Front);
				}
				else
				{
					RendererBase::SetCullMode(CullMode::Back);
				}
				Renderer::Submit(&mesh.Mesh, Renderer::GetShaderLibrary().Get("PBR-Shader").get(), c, mrao[0], mrao[1], (uint32_t)entity, transformMatrix);
			}
		}
		//2D Rendering
		Renderer2D::BeginScene(camera);
		{
			auto group = m_Registry.group<TransformComponent>(entt::get<SpriteRendererComponent>);
			for (auto& entity : group)
			{
				auto [transform, sprite] = group.get<TransformComponent, SpriteRendererComponent>(entity);
				const auto& transformMatrix = transform.GetTransform();
				if ((transform.NumNegativeScaleComps % 2))
				{
					RendererBase::SetCullMode(CullMode::Front);
				}
				else
				{
					RendererBase::SetCullMode(CullMode::Back);
				}
				Renderer2D::DrawSprite(transformMatrix, sprite, (int)entity);
			}
		}
		Renderer2D::EndScene();
	}
	void Scene::OnUpdateRuntime(float delta)
	{
		{
			m_Registry.view<NativeScriptComponent>().each([=](auto entity, auto& nsc) {
				if (!nsc.Instance) {
					nsc.InstantiateFunction();
					nsc.Instance->m_Entity = { entity, this };
					nsc.OnCreateFunction(nsc.Instance);
				}
			nsc.OnUpdateFunction(nsc.Instance, delta);
				});
		}
		//TO-DO: Use Rigid-Body Transform Inverse for Camera Components
		//Negative Translation * Rotation Transposed
		RuntimeCamera* mainCamera = nullptr;
		DirectX::XMMATRIX cameraTransform = DirectX::XMMatrixIdentity();
		{
			auto group = m_Registry.view<TransformComponent, CameraComponent>();
			for (auto entity : group)
			{
				auto [transform, camera] = group.get<TransformComponent, CameraComponent>(entity);
				if (camera.Primary)
				{
					mainCamera = &camera.camera;
					cameraTransform = transform.GetTransform();
					
					break;
				}
			}
		}
		if (mainCamera) {
			Renderer::BeginScene(mainCamera, cameraTransform);
			//3D Rendering
			{
				auto group = m_Registry.view<TransformComponent, MeshRendererComponent>();
				for (auto& entity : group)
				{
					//TO-DO MATERIALS
					static float color[4] = { 0.97f,0.323f,0.271f,1.f };
					float c[4]{
					std::pow(color[0], 2.2),
					std::pow(color[1], 2.2),
					std::pow(color[2], 2.2),
					std::pow(color[3], 2.2),
					};
					auto [transform, mesh] = group.get<TransformComponent, MeshRendererComponent>(entity);
					const auto& transformMatrix = transform.GetTransform();
					if ((transform.NumNegativeScaleComps % 2))
					{
						RendererBase::SetCullMode(CullMode::Front);
					}
					else
					{
						RendererBase::SetCullMode(CullMode::Back);
					}
					Renderer::Submit(&mesh.Mesh, Renderer::GetShaderLibrary().Get("PBR-Shader").get(), c, 0.f, 0.5f, 0.5f, transformMatrix);
				}
			}
			//2D Rendering
			Renderer2D::BeginScene(*mainCamera, cameraTransform);
			{
				auto group = m_Registry.group<TransformComponent>(entt::get<SpriteRendererComponent>);
				for (auto& entity : group)
				{
					auto [transform, sprite] = group.get<TransformComponent, SpriteRendererComponent>(entity);
					const auto& transformMatrix = transform.GetTransform();
					if ((transform.NumNegativeScaleComps % 2))
					{
						RendererBase::SetCullMode(CullMode::Front);
					}
					else
					{
						RendererBase::SetCullMode(CullMode::Back);
					}
					Renderer2D::DrawQuad(transformMatrix, sprite.Color);
				}
			}
			Renderer2D::EndScene();
		}
		
		
	}
	void Scene::OnViewportResize(unsigned int width, unsigned int height)
	{
		m_viewportWidth = width;
		m_ViewportHeight = height;
		auto view = m_Registry.view<CameraComponent>();
		for (auto entity : view)
		{
			auto& camera = view.get<CameraComponent>(entity);
			if (!camera.FixedAspectRatio)
			{
				camera.camera.SetViewportSize(width, height);
			}
		}
	}
	template<typename T>
	void OnComponentAdded(Entity entity, T& component)
	{
		//static_assert(false);
	}
	template<>
	void Scene::OnComponentAdded<TransformComponent>(Entity entity, TransformComponent& component)
	{
	}
	template<>
	void Scene::OnComponentAdded<TagComponent>(Entity entity, TagComponent& component)
	{
	}
	template<>
	void Scene::OnComponentAdded<CameraComponent>(Entity entity, CameraComponent& component)
	{
		component.camera.SetViewportSize(m_viewportWidth, m_ViewportHeight);
	}
	template<>
	void Scene::OnComponentAdded<SpriteRendererComponent>(Entity entity, SpriteRendererComponent& component)
	{
	}
	template<>
	void Scene::OnComponentAdded<MeshRendererComponent>(Entity entity, MeshRendererComponent& component)
	{
	}
	template<>
	void Scene::OnComponentAdded<NativeScriptComponent>(Entity entity, NativeScriptComponent& component)
	{
	}
	template<>
	void Scene::OnComponentAdded<LightComponent>(Entity entity, LightComponent& component)
	{
	}
}