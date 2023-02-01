#include "ptpch.h"
#include "Scene.h"
#include "Components.h"
#include "Pistachio/Renderer/Renderer2D.h"
#include "Pistachio/Renderer/Renderer.h"
#include "Entity.h"
#include "imgui.h"
#include "ImGuizmo.h"
#include "PxPhysicsAPI.h"
#include "ScriptableComponent.h"
class ErrorCallBack : public physx::PxErrorCallback
{
	virtual void reportError(physx::PxErrorCode::Enum code, const char* message, const char* file, int line) override
	{
		// error processing implementation
		PT_CORE_INFO("{0}", message);
	}
};

static physx::PxDefaultAllocator	gAllocator;
static ErrorCallBack gErrorCallback;
static physx::PxFoundation* gFoundation = NULL;
static physx::PxPhysics* gPhysics = NULL;
static physx::PxDefaultCpuDispatcher* gDispatcher = NULL;
static physx::PxPvd* gPvd = NULL;
static physx::PxMaterial* gMaterial = NULL;
namespace Pistachio {
	Scene::Scene()
	{
		gFoundation = PxCreateFoundation(PX_PHYSICS_VERSION, gAllocator, gErrorCallback);

		gPvd = PxCreatePvd(*gFoundation);
		physx::PxPvdTransport* transport = physx::PxDefaultPvdSocketTransportCreate("192.168.8.1", 5425, 10);
		gPvd->connect(*transport, physx::PxPvdInstrumentationFlag::eALL);

		gPhysics = PxCreatePhysics(PX_PHYSICS_VERSION, *gFoundation, physx::PxTolerancesScale(), true, gPvd);

		

	}
	Scene::~Scene()
	{
	}
	Entity Scene::CreateEntity(const std::string& name)
	{
		return CreateEntityWithUUID(UUID(), name);
	}
	Entity Scene::CreateEntityWithUUID(UUID ID, const std::string& name)
	{
		Entity entity = { m_Registry.create(), this };
		entity.AddComponent<IDComponent>(ID);
		entity.AddComponent<TransformComponent>();
		auto& tag = entity.AddComponent<TagComponent>();
		tag.Tag = name.empty() ? "Entity" : name;
		return entity;
	}
	void Scene::OnRuntimeStart()
	{
		physx::PxSceneDesc sceneDesc(gPhysics->getTolerancesScale());
		sceneDesc.gravity = physx::PxVec3(0.0f, -9.81f, 0.0f);
		gDispatcher = physx::PxDefaultCpuDispatcherCreate(2);
		sceneDesc.cpuDispatcher = gDispatcher;
		sceneDesc.filterShader = physx::PxDefaultSimulationFilterShader;
		m_PhysicsScene = gPhysics->createScene(sceneDesc);

		physx::PxPvdSceneClient* pvdClient = m_PhysicsScene->getScenePvdClient();
		if (pvdClient)
		{
			pvdClient->setScenePvdFlag(physx::PxPvdSceneFlag::eTRANSMIT_CONSTRAINTS, true);
			pvdClient->setScenePvdFlag(physx::PxPvdSceneFlag::eTRANSMIT_CONTACTS, true);
			pvdClient->setScenePvdFlag(physx::PxPvdSceneFlag::eTRANSMIT_SCENEQUERIES, true);
		}
		auto view = m_Registry.view<RigidBodyComponent>();
		for (auto e : view)
		{
			Entity entity = { e, this };
			auto& transform = entity.GetComponent<TransformComponent>();
			auto& rigidBody = entity.GetComponent<RigidBodyComponent>();
			if (rigidBody.type == RigidBodyComponent::BodyType::Dynamic)
			{
				physx::PxQuat rotation(DirectX::XMVectorGetX(transform.Rotation), DirectX::XMVectorGetY(transform.Rotation), DirectX::XMVectorGetZ(transform.Rotation), DirectX::XMVectorGetW(transform.Rotation));
				physx::PxTransform pxtransform(DirectX::XMVectorGetX(transform.Translation), DirectX::XMVectorGetY(transform.Translation), DirectX::XMVectorGetZ(transform.Translation), rotation);
				rigidBody.RuntimeBody = gPhysics->createRigidDynamic(pxtransform);
				physx::PxRigidBodyExt::updateMassAndInertia(*((physx::PxRigidBody*)rigidBody.RuntimeBody), rigidBody.Density);
				m_PhysicsScene->addActor(*((physx::PxRigidDynamic*)rigidBody.RuntimeBody));
			}
			else if (rigidBody.type == RigidBodyComponent::BodyType::Static)
			{
				physx::PxQuat rotation(DirectX::XMVectorGetX(transform.Rotation), DirectX::XMVectorGetY(transform.Rotation), DirectX::XMVectorGetZ(transform.Rotation), DirectX::XMVectorGetW(transform.Rotation));
				physx::PxTransform pxtransform(DirectX::XMVectorGetX(transform.Translation), DirectX::XMVectorGetY(transform.Translation), DirectX::XMVectorGetZ(transform.Translation), rotation);
				rigidBody.RuntimeBody = gPhysics->createRigidStatic(pxtransform);
				m_PhysicsScene->addActor(*((physx::PxRigidStatic*)rigidBody.RuntimeBody));
			}

			if (entity.HasComponent<BoxColliderComponent>())
			{
				auto& bc = entity.GetComponent<BoxColliderComponent>();
				gMaterial = gPhysics->createMaterial(bc.StaticFriction, bc.DynamicFriction, bc.Restitution);
				physx::PxShape* shape = gPhysics->createShape(physx::PxBoxGeometry(bc.size.x * DirectX::XMVectorGetX(transform.Scale), bc.size.y * DirectX::XMVectorGetY(transform.Scale), bc.size.z * DirectX::XMVectorGetZ(transform.Scale)), *gMaterial);
				physx::PxTransform pose(bc.offset.x, bc.offset.y, bc.offset.z);
				shape->setLocalPose(pose);
				((physx::PxRigidActor*)rigidBody.RuntimeBody)->attachShape(*shape);
				gMaterial->release();
			}
			if (entity.HasComponent<SphereColliderComponent>())
			{
				auto& bc = entity.GetComponent<SphereColliderComponent>();
				gMaterial = gPhysics->createMaterial(.5f, .5f, .6f);
				physx::PxShape* shape = gPhysics->createShape(physx::PxSphereGeometry(1.f), *gMaterial);
				physx::PxTransform pose(bc.offset.x, bc.offset.y, bc.offset.z);
				shape->setLocalPose(pose);
				((physx::PxRigidActor*)rigidBody.RuntimeBody)->attachShape(*shape);
				gMaterial->release();
			}
		}
	}
	void Scene::OnRuntimeStop()
	{
		m_PhysicsScene->release();
		m_PhysicsScene = nullptr;
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
				auto [transform, mesh] = group.get<TransformComponent, MeshRendererComponent>(entity);
				float c[4]{
				std::pow(mesh.color.x, 2.2),
				std::pow(mesh.color.y, 2.2),
				std::pow(mesh.color.z, 2.2),
				};
				const auto& transformMatrix = transform.GetTransform();
				if ((transform.NumNegativeScaleComps % 2))
				{
					RendererBase::SetCullMode(CullMode::Front);
				}
				else
				{
					RendererBase::SetCullMode(CullMode::Back);
				}
				if (mesh.Mesh)
				Renderer::Submit(mesh.Mesh.get(), Renderer::GetShaderLibrary().Get("PBR-Shader").get(), c,mesh.metallic, mesh.roughness, (uint32_t)entity, transformMatrix);
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
		{
			m_PhysicsScene->simulate(delta);
			m_PhysicsScene->fetchResults(true);
			auto view = m_Registry.view<RigidBodyComponent>();
			for (auto e : view) {
				Entity entity = { e, this };
				auto& tc = entity.GetComponent<TransformComponent>();
				auto& rigidBody = entity.GetComponent<RigidBodyComponent>();
				physx::PxTransform transform = ((physx::PxRigidActor*)rigidBody.RuntimeBody)->getGlobalPose();
				// roll (x-axis rotation)
				double sinr_cosp = 2 * (transform.q.w * transform.q.x + transform.q.y * transform.q.z);
				double cosr_cosp = 1 - 2 * (transform.q.x * transform.q.x + transform.q.y * transform.q.y);
				float roll = std::atan2(sinr_cosp, cosr_cosp);

				// pitch (y-axis rotation)
				double sinp = std::sqrt(1 + 2 * (transform.q.w * transform.q.y - transform.q.x * transform.q.z));
				double cosp = std::sqrt(1 - 2 * (transform.q.w * transform.q.y - transform.q.x * transform.q.z));
				float pitch = 2 * std::atan2(sinp, cosp) - 3.1415927 / 2;

				// yaw (z-axis rotation)
				double siny_cosp = 2 * (transform.q.w * transform.q.z + transform.q.x * transform.q.y);
				double cosy_cosp = 1 - 2 * (transform.q.y * transform.q.y + transform.q.z * transform.q.z);
				float yaw = std::atan2(siny_cosp, cosy_cosp);
				tc.RotationEulerHint = { roll, pitch, yaw, 1.f };
				tc.Translation = DirectX::XMVectorSet(transform.p.x, transform.p.y, transform.p.z, 1.f);
				tc.Rotation = DirectX::XMVectorSet(transform.q.x, transform.q.y, transform.q.z, transform.q.w);
			}
		}
		{
			auto group = m_Registry.view<TransformComponent, LightComponent>();
			for (auto& entity : group)
			{
				Light light;
				auto [tc, lightcomponent] = group.get<TransformComponent, LightComponent>(entity);
				if (lightcomponent.Type == 0) {
					DirectX::XMVECTOR lightTransform = DirectX::XMVector3Rotate(DirectX::XMVectorSet(0.f, 1.f, 0.f, 1.f), DirectX::XMQuaternionRotationRollPitchYawFromVector(tc.Rotation));
					light.positionxtype = { DirectX::XMVectorGetX(lightTransform), DirectX::XMVectorGetY(lightTransform), DirectX::XMVectorGetZ(lightTransform), 0 };
				}
				else if (lightcomponent.Type == 1)
					light.positionxtype = { DirectX::XMVectorGetX(tc.Translation), DirectX::XMVectorGetY(tc.Translation), DirectX::XMVectorGetZ(tc.Translation), 1 };
				light.colorxintensity = { lightcomponent.color.x, lightcomponent.color.y, lightcomponent.color.z, lightcomponent.Intensity };
				Renderer::AddLight(light);
			}
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
					auto [transform, mesh] = group.get<TransformComponent, MeshRendererComponent>(entity);
					float c[4]{
						std::pow(mesh.color.x, 2.2),
						std::pow(mesh.color.y, 2.2),
						std::pow(mesh.color.z, 2.2),
					};
					const auto& transformMatrix = transform.GetTransform();
					if ((transform.NumNegativeScaleComps % 2))
					{
						RendererBase::SetCullMode(CullMode::Front);
					}
					else
					{
						RendererBase::SetCullMode(CullMode::Back);
					}
					if(mesh.Mesh)
					Renderer::Submit(mesh.Mesh.get(), Renderer::GetShaderLibrary().Get("PBR-Shader").get(), c, mesh.metallic, mesh.roughness, 0.5f, transformMatrix);
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
	void Scene::OnComponentAdded<IDComponent>(Entity entity, IDComponent& component)
	{
	}
	template<>
	void Scene::OnComponentAdded<LightComponent>(Entity entity, LightComponent& component)
	{
	}
	template<>
	void Scene::OnComponentAdded<RigidBodyComponent>(Entity entity, RigidBodyComponent& component)
	{
	}
	template<>
	void Scene::OnComponentAdded<BoxColliderComponent>(Entity entity, BoxColliderComponent& component)
	{
	}
	template<>
	void Scene::OnComponentAdded<SphereColliderComponent>(Entity entity, SphereColliderComponent& component)
	{
	}
}