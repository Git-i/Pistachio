#include "ptpch.h"
#include "Scene.h"
#include "Components.h"
#include "Pistachio/Renderer/Renderer2D.h"

#include "Entity.h"
#include "imgui.h"
#include "ImGuizmo.h"
#include "PxPhysicsAPI.h"
#include "ScriptableComponent.h"
#include "Pistachio/Physics/Physics.h"
#include "Pistachio/Renderer/MeshFactory.h"
#include "CullingManager.h"

static void getFrustumCornersWorldSpace(const DirectX::XMMATRIX& proj, const DirectX::XMMATRIX& view, DirectX::XMVECTOR* corners)
{
	PT_PROFILE_FUNCTION();
	const auto inv = DirectX::XMMatrixInverse(nullptr, view * proj);
	int index = 0;
	for (unsigned int x = 0; x < 2; ++x)
	{
		for (unsigned int y = 0; y < 2; ++y)
		{
			for (unsigned int z = 0; z < 2; ++z)
			{
				auto pt = DirectX::XMVector4Transform(DirectX::XMVectorSet(2.0f * x - 1.0f, 2.0f * y - 1.0f, 2.0f * z - 1.0f, 1.0f), inv);
				corners[index] = DirectX::XMVectorScale(pt, 1.f/DirectX::XMVectorGetW(pt));
				index++;
			}
		}
	}
}
static DirectX::XMMATRIX GetLightMatrixFromCamera(const DirectX::XMMATRIX& camView, const DirectX::XMMATRIX& camProj, const Pistachio::Light& light, float zMult)
{
	PT_PROFILE_FUNCTION();
	DirectX::XMVECTOR corners[8];
	getFrustumCornersWorldSpace(camProj, camView, corners);
	DirectX::XMVECTOR center = DirectX::XMVectorZero();
	for (const auto& v : corners)
	{
		center = DirectX::XMVectorAdd(v, center);
	}
	center = DirectX::XMVectorScale(center, 1.f / 8.f);

	const auto lightView = DirectX::XMMatrixLookAtLH(DirectX::XMVectorAdd(center, DirectX::XMLoadFloat4(&light.rotation)),
		center,
		DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 1.f));
	float minX = std::numeric_limits<float>::max();
	float maxX = std::numeric_limits<float>::lowest();
	float minY = std::numeric_limits<float>::max();
	float maxY = std::numeric_limits<float>::lowest();
	float minZ = std::numeric_limits<float>::max();
	float maxZ = std::numeric_limits<float>::lowest();
	for (const auto& v : corners)
	{
		DirectX::XMVECTOR trf_xmv = DirectX::XMVector4Transform(v, lightView);
		DirectX::XMFLOAT3 trf;
		DirectX::XMStoreFloat3(&trf, trf_xmv);
		minX = std::min(minX, trf.x);
		maxX = std::max(maxX, trf.x);
		minY = std::min(minY, trf.y);
		maxY = std::max(maxY, trf.y);
		minZ = std::min(minZ, trf.z);
		maxZ = std::max(maxZ, trf.z);
	}
	if (minZ < 0)
	{
		minZ *= zMult;
	}
	else
	{
		minZ /= zMult;
	}
	if (maxZ < 0)
	{
		maxZ /= zMult;
	}
	else
	{
		maxZ *= zMult;
	}

	const DirectX::XMMATRIX lightProjection = DirectX::XMMatrixOrthographicOffCenterLH(minX, maxX, minY, maxY, minZ, maxZ);
	return DirectX::XMMatrixMultiplyTranspose(lightView, lightProjection);
}
static D3D11_VIEWPORT vp[4];
static void ChangeVP(float size, Pistachio::hiVector2 offset)
{
	vp[0].Width = vp[0].Height = size;
	vp[0].TopLeftX = offset.x;
	vp[0].TopLeftY = offset.y;
	vp[1] = vp[2] = vp[3] = vp[0];
	vp[1].TopLeftX += size;
	vp[2].TopLeftY += size;
	vp[3].TopLeftX += size;
	vp[3].TopLeftY += size;
}
namespace Pistachio {
	
	Scene::Scene(SceneDesc desc) : sm_allocator({ 4096, 4096 }, {256, 256})
	{
		PT_PROFILE_FUNCTION();
		CreateEntity("Root").GetComponent<ParentComponent>().parentID = -1;
		vp[0].TopLeftX = 0;
		vp[0].TopLeftY = 0;
		vp[0].Width = 2048;
		vp[0].Height = 2048;
		vp[0].MinDepth = 0;
		vp[0].MaxDepth = 1;
		vp[1] = vp[2] = vp[3] = vp[0];
		vp[1].TopLeftX = 2048;
		vp[2].TopLeftY = 2048;
		vp[3].TopLeftX = vp[3].TopLeftY = 2048;
		RenderTextureDesc rtDesc;
		rtDesc.width = desc.Resolution.x;
		rtDesc.height = desc.Resolution.y;
		rtDesc.miplevels = 1;
		rtDesc.Attachments = { TextureFormat::RGBA8U,  TextureFormat::RGBA16F,TextureFormat::RGBA16F, TextureFormat::RGBA16F, TextureFormat::D24S8 };
		m_gBuffer.CreateStack(rtDesc);
		RenderTextureDesc rtDesc2;
		rtDesc2.width = desc.Resolution.x;
		rtDesc2.height = desc.Resolution.y;
		rtDesc2.miplevels = 1;
		rtDesc2.Attachments = { TextureFormat::RGBA8U, TextureFormat::D24S8 };
		m_finalRender.CreateStack(rtDesc2);
		ScreenSpaceQuad = MeshFactory::CreatePlane();
	}
	Scene::~Scene()
	{
		delete ScreenSpaceQuad;
	}
	Entity Scene::CreateEntity(const std::string& name)
	{
		return CreateEntityWithUUID(UUID(), name);
	}
	Entity Scene::DuplicateEntity(Entity entity)
	{
		if (entity.GetComponent<ParentComponent>().parentID == -1)
			return entity;
		Entity newEntity = CreateEntity(entity.GetComponent<TagComponent>().Tag + "-Copy");
		newEntity.GetComponent<TransformComponent>() = entity.GetComponent<TransformComponent>();
		newEntity.GetComponent<ParentComponent>() = entity.GetComponent<ParentComponent>();
		if (entity.HasComponent<LightComponent>()) newEntity.AddComponent<LightComponent>(entity.GetComponent<LightComponent>());
		if (entity.HasComponent<SpriteRendererComponent>()) newEntity.AddComponent<SpriteRendererComponent>() = entity.GetComponent<SpriteRendererComponent>();
		if (entity.HasComponent<MeshRendererComponent>()) newEntity.AddComponent<MeshRendererComponent>() = entity.GetComponent<MeshRendererComponent>();
		if (entity.HasComponent<RigidBodyComponent>()) newEntity.AddComponent<RigidBodyComponent>() = entity.GetComponent<RigidBodyComponent>();
		if (entity.HasComponent<BoxColliderComponent>()) newEntity.AddComponent<BoxColliderComponent>() = entity.GetComponent<BoxColliderComponent>();
		if (entity.HasComponent<SphereColliderComponent>()) newEntity.AddComponent<SphereColliderComponent>() = entity.GetComponent<SphereColliderComponent>();
		return newEntity;
	}
	Entity Scene::CreateEntityWithUUID(UUID ID, const std::string& name)
	{
		Entity entity = { m_Registry.create(), this };
		entity.AddComponent<IDComponent>(ID);
		entity.AddComponent<ParentComponent>(0);
		entity.AddComponent<TransformComponent>();
		auto& tag = entity.AddComponent<TagComponent>();
		char id[100] = {'E','n','t','i','t', 'y', '0', '0', '0', '\0'};
		_itoa_s((uint32_t)entity, &id[6], 20, 10);
		tag.Tag = name.empty() ? id : name;
		return entity;
	}
	void Scene::OnRuntimeStart()
	{
		physx::PxSceneDesc sceneDesc(Physics::gPhysics->getTolerancesScale());
		sceneDesc.gravity = physx::PxVec3(0.0f, -9.81f, 0.0f);
		Physics::gDispatcher = physx::PxDefaultCpuDispatcherCreate(2);
		sceneDesc.cpuDispatcher = Physics::gDispatcher;
		sceneDesc.filterShader = physx::PxDefaultSimulationFilterShader;
		m_PhysicsScene = Physics::gPhysics->createScene(sceneDesc);

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
			auto& rigidBody = view.get<RigidBodyComponent>(e);
			Physics::gMaterial = Physics::gPhysics->createMaterial(rigidBody.StaticFriction, rigidBody.DynamicFriction, rigidBody.Restitution);
			if (rigidBody.type == RigidBodyComponent::BodyType::Dynamic)
			{
				physx::PxQuat rotation(DirectX::XMVectorGetX(transform.Rotation), DirectX::XMVectorGetY(transform.Rotation), DirectX::XMVectorGetZ(transform.Rotation), DirectX::XMVectorGetW(transform.Rotation));
				physx::PxTransform pxtransform(DirectX::XMVectorGetX(transform.Translation), DirectX::XMVectorGetY(transform.Translation), DirectX::XMVectorGetZ(transform.Translation), rotation);
				rigidBody.RuntimeBody = Physics::gPhysics->createRigidDynamic(pxtransform);
				physx::PxRigidBodyExt::updateMassAndInertia(*((physx::PxRigidBody*)rigidBody.RuntimeBody), rigidBody.Density);
				m_PhysicsScene->addActor(*((physx::PxRigidDynamic*)rigidBody.RuntimeBody));
			}
			else if (rigidBody.type == RigidBodyComponent::BodyType::Static)
			{
				physx::PxQuat rotation(DirectX::XMVectorGetX(transform.Rotation), DirectX::XMVectorGetY(transform.Rotation), DirectX::XMVectorGetZ(transform.Rotation), DirectX::XMVectorGetW(transform.Rotation));
				physx::PxTransform pxtransform(DirectX::XMVectorGetX(transform.Translation), DirectX::XMVectorGetY(transform.Translation), DirectX::XMVectorGetZ(transform.Translation), rotation);
				rigidBody.RuntimeBody = Physics::gPhysics->createRigidStatic(pxtransform);
				m_PhysicsScene->addActor(*((physx::PxRigidStatic*)rigidBody.RuntimeBody));
			}

			if (entity.HasComponent<BoxColliderComponent>())
			{
				auto& bc = entity.GetComponent<BoxColliderComponent>();
				physx::PxShape* shape = Physics::gPhysics->createShape(physx::PxBoxGeometry(bc.size.x * DirectX::XMVectorGetX(transform.Scale), bc.size.y * DirectX::XMVectorGetY(transform.Scale), bc.size.z * DirectX::XMVectorGetZ(transform.Scale)), *Physics::gMaterial);
				physx::PxTransform pose(bc.offset.x, bc.offset.y, bc.offset.z);
				shape->setLocalPose(pose);
				((physx::PxRigidActor*)rigidBody.RuntimeBody)->attachShape(*shape);
			}
			if (entity.HasComponent<SphereColliderComponent>())
			{
				auto& sc = entity.GetComponent<SphereColliderComponent>();
				physx::PxShape* shape = Physics::gPhysics->createShape(physx::PxSphereGeometry(sc.size), *Physics::gMaterial);
				physx::PxTransform pose(sc.offset.x, sc.offset.y, sc.offset.z);
				shape->setLocalPose(pose);
				((physx::PxRigidActor*)rigidBody.RuntimeBody)->attachShape(*shape);
			}
			if (entity.HasComponent<CapsuleColliderComponent>())
			{
				auto& cc = entity.GetComponent<CapsuleColliderComponent>();
				physx::PxShape* shape = Physics::gPhysics->createShape(physx::PxCapsuleGeometry(cc.radius, cc.height), *Physics::gMaterial);
				physx::PxTransform pose(cc.offset.x, cc.offset.y, cc.offset.z);
				shape->setLocalPose(pose);
				((physx::PxRigidActor*)rigidBody.RuntimeBody)->attachShape(*shape);
			}
			if (entity.HasComponent<PlaneColliderComponent>())
			{
				auto& pc = entity.GetComponent<PlaneColliderComponent>();
				physx::PxShape* shape = Physics::gPhysics->createShape(physx::PxPlaneGeometry(), *Physics::gMaterial);
				physx::PxTransform pose(pc.offset.x, pc.offset.y, pc.offset.z);
				shape->setLocalPose(pose);
				((physx::PxRigidActor*)rigidBody.RuntimeBody)->attachShape(*shape);
			}
			Physics::gMaterial->release();
		}
	}
	void Scene::OnRuntimeStop()
	{
		m_PhysicsScene->release();
		m_PhysicsScene = nullptr;
	}
	void Scene::DestroyEntity(Entity entity)
	{
		auto view = m_Registry.view<ParentComponent>();
		for (auto child : view)
		{
			if (view.get<ParentComponent>(child).parentID == (std::uint32_t)entity)
				m_Registry.destroy(child);
		}
		m_Registry.destroy(entity);
	}
	Entity Scene::GetRootEntity()
	{
		
		auto view = m_Registry.view<ParentComponent>();
		for (auto entity : view)
		{
			if (view.get<ParentComponent>(entity).parentID == -1)
				return Entity(entity, this);
		}
		return Entity();
	}
	Entity Scene::GetPrimaryCameraEntity()
	{
		PT_PROFILE_FUNCTION();
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
		PT_PROFILE_FUNCTION();
		
		/*
		  1. Gather a list of lights that affect the frame (culling), if a light doesnt affect a frame make sure its region in the atlas is invalid.
		  2. for any light that already has a region, we check the cascade from camera to see if it needs to be shrunk or grown, then render all meshes to the atlas if the region was shrunk or grown 
		  3. if any light in the list doesnt have an atlas region, assign one to it based on distance from camera, and render all meshes it affects into the atlas.
		  4. for every light that already has a region and didn't change cascade (same atlas region) we check all the meshes it affects if any are "dirty", if so we re render all meshes in that region
		  5. proceed with clustered forward shading, using our list if culled lights and our shadow atlas

		  After Sorting Mesh Components Of Course
		*/
		//todo add a quality setting features
		static std::vector<ShadowCastingLight> shadowCastingLights;
		static std::vector<bool> shadow_dirty; //todo: do this better
		static std::vector<RegularLight> regularLights;
		auto transformMesh = m_Registry.view<TransformComponent, MeshRendererComponent>();
		static std::vector<Entity> meshesToDraw;
		//mark dirty transforms
		{
			PT_PROFILE_SCOPE("Dirty Transform Components");
			auto transformParent = m_Registry.view<ParentComponent, TransformComponent>();
			for (auto entity : transformParent)
			{
				auto [Parent, transform] = transformParent.get(entity);
				if (transform.bDirty)
				{
					if (Parent.parentID >= 0)
						transform.worldSpaceTransform = transform.GetTransform({ (entt::entity)Parent.parentID, this });
					else
						transform.worldSpaceTransform = transform.GetLocalTransform();
				}
				auto PID = Parent.parentID;
				while (PID != -1)
				{
					auto& parentTransform = m_Registry.get<TransformComponent>((entt::entity)PID);
					if (parentTransform.bDirty)
					{
						transform.bDirty = true;
						if (Parent.parentID >= 0)
							transform.worldSpaceTransform = transform.GetTransform({ (entt::entity)Parent.parentID, this });
						else
							transform.worldSpaceTransform = transform.GetLocalTransform();
						break;
					}
					auto& parentComp = m_Registry.get<ParentComponent>((entt::entity)PID);
					PID = parentComp.parentID;
				}
			}
		}
		//todo: combine these two functions into one that loops through all the meshes, and probably also does the frsutum culling
		UpdateObjectCBs();
		SortMeshComponents();//clean dirty transform meshes
		//frustum culling
		BoundingFrustum cameraFrustum(camera.GetProjection());
		cameraFrustum.Transform(cameraFrustum, camera.GetViewMatrix().Invert());
		{
			for (auto e : transformMesh)
			{
				auto [transform, mesh] = transformMesh.get(e);
				Model* model = GetAssetManager()->GetModelResource(mesh.Model);
				if (mesh.Model.m_uuid)
				{
					BoundingBox box = model->aabbs[mesh.modelIndex];
					box.Transform(box, transform.worldSpaceTransform);
					if (CullingManager::FrustumCull(box, cameraFrustum)) //we passed the test
					{
						meshesToDraw.emplace_back(e, this);
					}
					else
					{
						int a = 9;
					}
				}
			}
		}

		float color[4] = { 0,0,0,0 };
		m_gBuffer.ClearAll(color);
		m_finalRender.Clear(color, 0);
		
		{
			PT_PROFILE_SCOPE("Shadow Rendereing and Light Formation")
			Renderer::whiteTexture.Bind(9);
			auto group = m_Registry.view<TransformComponent, LightComponent>();
			
			for (auto& entity : group)
			{
				auto [tc, lightcomponent] = group.get(entity);
				if (lightcomponent.Type == LightType::Point || lightcomponent.Type == LightType::Spot)
				{
					bool visible = CullingManager::FrustumCull(BoundingSphere(tc.Translation, lightcomponent.exData.z), cameraFrustum);
					if (!visible)
					{
						if (lightcomponent.shadowMap)
						{
							//todo: remove this line, it is completely unnecessary and only for debug
							Renderer::shadowMapAtlas.Clear(sm_allocator.GetRegion(lightcomponent.shadowMap), 0.f);
							sm_allocator.DeAllocate(lightcomponent.shadowMap);
							lightcomponent.shadowMap = 0;
						}
						continue; //de allocate and dont submit to renderer
					}
				}
				Light light;
				DirectX::XMStoreFloat3(&light.position, tc.Translation);
				light.type = lightcomponent.Type;
				float z = 1.f;
				if (light.type == LightType::Directional) z = -1.f;
				DirectX::XMVECTOR lightTransform = DirectX::XMVector3Rotate(DirectX::XMVectorSet(0.f, 0.f, z, 1.f), tc.Rotation);
				DirectX::XMStoreFloat4(&light.rotation, lightTransform);
				light.exData = { lightcomponent.exData.x , lightcomponent.exData.y, lightcomponent.exData.z, (float)lightcomponent.shadow};
				light.color = { lightcomponent.color.x, lightcomponent.color.y, lightcomponent.color.z };
				light.intensity = { lightcomponent.Intensity };
				DirectX::XMMATRIX lightMatrix[4] = { DirectX::XMMatrixIdentity(),DirectX::XMMatrixIdentity(),DirectX::XMMatrixIdentity(),DirectX::XMMatrixIdentity() };
				if (lightcomponent.shadow)
				{
					iVector2 allocation_size;
					int numLightMatrices;
					bool dirty = lightcomponent.shadow_dirty;
					if(lightcomponent.Type == LightType::Directional)
					{
						dirty = true;
						numLightMatrices = 4;
						allocation_size = { 256*4, 256*4 };
						lightMatrix[0] = GetLightMatrixFromCamera(camera.GetViewMatrix(), DirectX::XMMatrixPerspectiveFovLH(DirectX::XMConvertToRadians(camera.GetFOVdeg()), camera.GetAspectRatio(), camera.GetNearClip(), 30.f), light, 1.05f);
						lightMatrix[1] = GetLightMatrixFromCamera(camera.GetViewMatrix(), DirectX::XMMatrixPerspectiveFovLH(DirectX::XMConvertToRadians(camera.GetFOVdeg()), camera.GetAspectRatio(), 30.f, 100.f), light, 1.2f);
						lightMatrix[2] = GetLightMatrixFromCamera(camera.GetViewMatrix(), DirectX::XMMatrixPerspectiveFovLH(DirectX::XMConvertToRadians(camera.GetFOVdeg()), camera.GetAspectRatio(), 100.f, 500.f), light, 1.2f);
						lightMatrix[3] = GetLightMatrixFromCamera(camera.GetViewMatrix(), DirectX::XMMatrixPerspectiveFovLH(DirectX::XMConvertToRadians(camera.GetFOVdeg()), camera.GetAspectRatio(), 500.f, camera.GetFarClip()), light, 1.2f);
					}
					else if (lightcomponent.Type == LightType::Spot)
					{
						numLightMatrices = 1;
						allocation_size = { 256 , 256 };
						lightMatrix[0] = DirectX::XMMatrixTranspose(DirectX::XMMatrixLookAtLH(tc.Translation, DirectX::XMVectorAdd(tc.Translation, DirectX::XMLoadFloat4(&light.rotation)), DirectX::XMVectorSet(0,1,0,0)) * DirectX::XMMatrixPerspectiveFovLH(DirectX::XMScalarACos(lightcomponent.exData.x) * 2, 1, 0.1f, lightcomponent.exData.z));
					}
					else if (lightcomponent.Type == LightType::Point)
					{
						int numLightMatrices = 4;
						allocation_size = {256*3, 256*2};
						//todo : Handle point light matrices;
					}
					Region sm_region;
					
					if (tc.bDirty)
					{
						dirty = true;
					}
					if (lightcomponent.shadowMap != 0) // if there was a shadow map dont allocate a new one unnecessarily
					{
						//todo camera cascades for varying shadow map sizes at different distance levels
						sm_region = sm_allocator.GetRegion(lightcomponent.shadowMap);
					}
					else
					{
						lightcomponent.shadowMap = sm_allocator.Allocate(allocation_size, AllocatorFlags::None); // todo render settings to control allocation size
						sm_region = sm_allocator.GetRegion(lightcomponent.shadowMap);
						dirty = true;
					}
					lightcomponent.shadow_dirty = false;
					shadow_dirty.emplace_back(dirty);
					Renderer::AddShadowCastingLight(shadowCastingLights.emplace_back(lightMatrix, sm_region, light, numLightMatrices));
				}
				else
				{
					if (lightcomponent.shadowMap != 0)
					{
						//todo: remove this line, it is completely unnecessary and only for debug
						Renderer::shadowMapAtlas.Clear(sm_allocator.GetRegion(lightcomponent.shadowMap), 0.f);
						sm_allocator.DeAllocate(lightcomponent.shadowMap);
						lightcomponent.shadowMap = 0;
					}
					Renderer::AddLight(regularLights.emplace_back(light));
				}
			}
		}
		//prepare for shadow map rendering
		if (!shadowCastingLights.empty())
		{
			
			Renderer::shadowMapAtlas.Bind();
			RendererBase::EnableShadowMapRasetrizerState();
			auto shader = Renderer::GetShaderLibrary().Get("Shadow-Shader");
			shader->Bind(ShaderType::Vertex);
			shader->Bind(ShaderType::Geometry);
			RendererBase::Getd3dDeviceContext()->PSSetShader(nullptr, nullptr, 0); //todo change this code
			
		}
		//dirty shadow casting lights, and draw dirty ones, directional lights will be handles spearately
		{
			int i = 0;
			for (auto& light : shadowCastingLights)
			{
				i++;
				BoundingSphere BoundingObject = {}; // todo: add support for frustum for more accurate spot light culling
				BoundingObject = BoundingSphere(light.light.position, light.light.exData.z);
				if (true)
				{
					Renderer::shadowMapAtlas.Clear(light.shadowMap);
					if(light.light.type == LightType::Spot) RenderSpotLightShadows(transformMesh, light);
					else if(light.light.type == LightType::Point) RenderPointLightShadows(transformMesh, light);
					else if(light.light.type == LightType::Directional) RenderDirectionalLightShadows(transformMesh, light);
					
				}
				//todo: improve efficieny of this code, i don't know how yet
				else
				{
					for (auto entity : transformMesh)//unsure if this would work
					{
						//frustum cull meshes, if they fall under the light, render all objects and continue the light loop
						auto [transform, mesh] = transformMesh.get(entity);


						if (transform.bDirty)
						{
							shadow_dirty[i-1] = true;
							bool visible = true;
							auto aabb = GetAssetManager()->GetModelResource(mesh.Model)->aabbs[mesh.modelIndex];
							aabb.Transform(aabb, transform.worldSpaceTransform);
							visible = CullingManager::SphereCull(aabb, BoundingObject);
							if (visible)
							{
								Renderer::shadowMapAtlas.Clear(light.shadowMap);
								if (light.light.type == LightType::Spot) RenderSpotLightShadows(transformMesh, light);
								else if (light.light.type == LightType::Point) RenderPointLightShadows(transformMesh, light);
								else if (light.light.type == LightType::Directional) RenderDirectionalLightShadows(transformMesh, light);
								break;
							}
						}
					}
				}
			
			}
		}
		if (!shadowCastingLights.empty())
		{
			RendererBase::SetCullMode(CullMode::Back);
			auto shader = Renderer::GetShaderLibrary().Get("GBuffer-Shader");
			shader->Bind(ShaderType::Pixel);
			shader->Bind(ShaderType::Vertex);
			RendererBase::Getd3dDeviceContext()->GSSetShader(nullptr, nullptr, 0); //todo change this code
		}
		
		m_gBuffer.Bind(0, 4);
		ImGui::Image(Renderer::shadowMapAtlas.m_SRV.Get(), { ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().x });
		Renderer::shadowMapAtlas.BindResource(9);
		//proceed with normal shading
		
		{
			PT_PROFILE_SCOPE("Object Rendering (Gbuffer Write)")
			Renderer::BeginScene(camera);
			int i = 0;
			for (auto& entity : meshesToDraw)
			{	
				i++;
				auto [transform, mesh] = transformMesh.get(entity);
				auto mat = GetAssetManager()->GetMaterialResource(mesh.material);
				auto model = GetAssetManager()->GetModelResource(mesh.Model);
				
				if (model) {
					Shader::SetVSBuffer(Renderer::TransformationBuffer[mesh.cbIndex], 1);
					if(!mat)
						Renderer::Submit(&model->meshes[mesh.modelIndex], Renderer::GetShaderLibrary().Get("GBuffer-Shader").get(), &Renderer::DefaultMaterial, (uint32_t)entity);
					else
						Renderer::Submit(&model->meshes[mesh.modelIndex], Renderer::GetShaderLibrary().Get("GBuffer-Shader").get(), mat, (uint32_t)entity);
				}
			}
			ImGui::Text("Meshes Rendered: %d", i);
		}
		m_finalRender.Bind(0, 1);
		m_gBuffer.BindResource(3, 4);
		Texture2D dst = m_finalRender.GetDepthTexture();
		Texture2D src = m_gBuffer.GetDepthTexture();
		auto& VB = ScreenSpaceQuad->GetVertexBuffer();
		auto& IB = ScreenSpaceQuad->GetIndexBuffer();
		Buffer buffer = { &VB,&IB };
		Renderer::GetShaderLibrary().Get("PBR-Deffered-Shader").get()->Bind(ShaderType::Vertex);
		Renderer::GetShaderLibrary().Get("PBR-Deffered-Shader").get()->Bind(ShaderType::Pixel);
		RendererBase::DrawIndexed(buffer);
		Renderer::whiteTexture.Bind(3);
		Renderer::whiteTexture.Bind(4);
		Renderer::whiteTexture.Bind(5);
		Renderer::whiteTexture.Bind(6);
		dst.CopyInto(src);
		//2D Rendering
		Renderer2D::BeginScene(camera);
		{
			PT_PROFILE_SCOPE("Drawing 2D Objects")
			auto transformSprite = m_Registry.view<TransformComponent, SpriteRendererComponent>();
			for (auto& entity : transformSprite)
			{
				auto [transform, sprite] = transformSprite.get(entity);
				const auto& transformMatrix = transform.GetTransform({ (entt::entity)m_Registry.get<ParentComponent>(entity).parentID, this });
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
		shadowCastingLights.clear();
		regularLights.clear();
		meshesToDraw.clear();
		shadow_dirty.clear();
		//clean transforms here
		auto transforms = m_Registry.view<TransformComponent>();
		for (auto e : transforms)
		{
			auto [transform] = transforms.get(e);
			transform.bDirty = false;
		}
	}
	void Scene::OnUpdateRuntime(float delta)
	{

	}
	/*void Scene::OnUpdateRuntime(float delta)
	{
		PT_PROFILE_FUNCTION();
		auto transfromMesh = m_Registry.view<TransformComponent, MeshRendererComponent>();
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

		
		//TO-DO: Use Rigid-Body Transform Inverse for Camera Components
		//Negative Translation * Rotation Transposed
		SceneCamera* mainCamera = nullptr;
		DirectX::XMMATRIX cameraTransform = DirectX::XMMatrixIdentity();
		{
			auto group = m_Registry.view<TransformComponent, CameraComponent>();
			for (auto entity : group)
			{
				auto [transform, camera] = group.get<TransformComponent, CameraComponent>(entity);
				if (camera.Primary)
				{
					mainCamera = &camera.camera;
					cameraTransform = transform.GetTransform({ (entt::entity)m_Registry.get<ParentComponent>(entity).parentID, this });
					
					break;
				}
			}
		}
		if (mainCamera) {
			SortMeshComponents();
			UpdateObjectCBs();
			float color[4] = { 0,0,0,0 };
			m_gBuffer.ClearAll(color);
			m_finalRender.Clear(color, 0);
			m_gBuffer.Bind(0, 4);
			DirectX::XMMATRIX view = DirectX::XMMatrixInverse(nullptr, cameraTransform);
			{
				Renderer::whiteTexture.Bind(9);
				auto group = m_Registry.view<TransformComponent, LightComponent>();
				for (auto& entity : group)
				{
					Light light;
					auto [tc, lightcomponent] = group.get<TransformComponent, LightComponent>(entity);
					DirectX::XMStoreFloat3(&light.position, tc.Translation);
					light.type = lightcomponent.Type;
					DirectX::XMVECTOR lightTransform = DirectX::XMVector3Rotate(DirectX::XMVectorSet(0.f, 0.f, -1.f, 1.f), tc.Rotation);
					DirectX::XMStoreFloat4(&light.rotation, lightTransform);
					light.exData = { lightcomponent.exData.x , lightcomponent.exData.y, lightcomponent.exData.z, (float)lightcomponent.CastShadow };
					light.colorxintensity = { lightcomponent.color.x, lightcomponent.color.y, lightcomponent.color.z, lightcomponent.Intensity };
					DirectX::XMMATRIX lightMatrix[4] = { DirectX::XMMatrixIdentity(),DirectX::XMMatrixIdentity(),DirectX::XMMatrixIdentity(),DirectX::XMMatrixIdentity() };
					if (lightcomponent.CastShadow)
					{
						RendererBase::SetCullMode(CullMode::Front);
						if (lightcomponent.Type == LightType::Directional)
						{
							float aspect = (float)m_viewportWidth / (float)m_ViewportHeight;
							lightMatrix[0] = GetLightMatrixFromCamera(view, DirectX::XMMatrixPerspectiveFovLH(mainCamera->GetPerspSize(), aspect, mainCamera->GetPerspNear(), 30.f), light, 1.05f);
							lightMatrix[1] = GetLightMatrixFromCamera(view, DirectX::XMMatrixPerspectiveFovLH(mainCamera->GetPerspSize(), aspect, 30.f, 100.f), light, 1.2f);
							lightMatrix[2] = GetLightMatrixFromCamera(view, DirectX::XMMatrixPerspectiveFovLH(mainCamera->GetPerspSize(), aspect, 100.f, 500.f), light, 1.f);
							lightMatrix[3] = GetLightMatrixFromCamera(view, DirectX::XMMatrixPerspectiveFovLH(mainCamera->GetPerspSize(), aspect, 500.f, mainCamera->GetPerspFar()), light, 1.f);
						}
						else if (lightcomponent.Type == LightType::Spot)
						{
							lightMatrix[0] = lightMatrix[1] = lightMatrix[2] = lightMatrix[3] = DirectX::XMMatrixTranspose(DirectX::XMMatrixLookAtLH(tc.Translation, DirectX::XMVectorAdd(tc.Translation, DirectX::XMLoadFloat4(&light.rotation)), DirectX::XMVectorSet(0, 1, 0, 0)) * DirectX::XMMatrixPerspectiveFovLH(DirectX::XMScalarACos(lightcomponent.exData.x) * 2, 1, 0.1f, lightcomponent.exData.z));
						}
						lightcomponent.shadowMap.Clear();
						lightcomponent.shadowMap.Bind();
						RendererBase::Getd3dDeviceContext()->PSSetShader(nullptr, nullptr, 0);
						Renderer::GetShaderLibrary().Get("Shadow-Shader")->Bind(ShaderType::Vertex);
						Renderer::GetShaderLibrary().Get("Shadow-Shader")->Bind(ShaderType::Geometry);
						Renderer::passConstants.lightSpaceMatrix[((int)Renderer::passConstants.numlights.x * 4) + 0] = lightMatrix[0];
						Renderer::passConstants.lightSpaceMatrix[((int)Renderer::passConstants.numlights.x * 4) + 1] = lightMatrix[1];
						Renderer::passConstants.lightSpaceMatrix[((int)Renderer::passConstants.numlights.x * 4) + 2] = lightMatrix[2];
						Renderer::passConstants.lightSpaceMatrix[((int)Renderer::passConstants.numlights.x * 4) + 3] = lightMatrix[3];
						Renderer::passConstants.EyePosW.w = lightcomponent.shadowMap.GetSize();
						ChangeVP(lightcomponent.shadowMap.GetSize() / 2);
						RendererBase::Getd3dDeviceContext()->RSSetViewports(4, vp);
						Renderer::UpdatePassConstants();
						for (auto& entity : group)
						{
							//TO-DO MATERIALS
							auto [transform, mesh] = transfromMesh.get(entity);
							auto model = GetAssetManager()->GetModelResource(mesh.Model);
							if (model) {
								auto& VB = model->meshes[mesh.modelIndex].GetVertexBuffer();
								auto& IB = model->meshes[mesh.modelIndex].GetIndexBuffer();
								Buffer buffer = { &VB,&IB };
								RendererBase::DrawIndexed(buffer);
							}
						}
						RendererBase::ChangeViewport(m_gBuffer.GetWidth(), m_gBuffer.GetHeight());
						m_gBuffer.Bind(0, 4);
						lightcomponent.shadowMap.BindResource(9 + Renderer::passConstants.numlights.x);
						RendererBase::SetCullMode(CullMode::Back);
					}
					Renderer::AddLight(light);
					RendererBase::Getd3dDeviceContext()->GSSetShader(nullptr, nullptr, 0);
				}
			}
			Renderer::BeginScene(mainCamera, cameraTransform);
			//3D Rendering
			{
				for (auto& entity : transfromMesh)
				{
					auto [transform, mesh] = transfromMesh.get(entity);
					auto mat = GetAssetManager()->GetMaterialResource(mesh.material);
					auto model = GetAssetManager()->GetModelResource(mesh.Model);
					if (model) {

						Shader::SetVSBuffer(Renderer::TransformationBuffer[mesh.cbIndex], 1);
						if (!mat)
							Renderer::Submit(&model->meshes[mesh.modelIndex], Renderer::GetShaderLibrary().Get("GBuffer-Shader").get(), &Renderer::DefaultMaterial, (uint32_t)entity);
						else
							Renderer::Submit(&model->meshes[mesh.modelIndex], Renderer::GetShaderLibrary().Get("GBuffer-Shader").get(), mat, (uint32_t)entity);
					}
				}
			}
			m_finalRender.Bind(0, 1);
			m_gBuffer.BindResource(3, 4);
			Texture2D dst = m_finalRender.GetDepthTexture();
			Texture2D src = m_gBuffer.GetDepthTexture();
			auto& VB = ScreenSpaceQuad->GetVertexBuffer();
			auto& IB = ScreenSpaceQuad->GetIndexBuffer();
			Buffer buffer = { &VB,&IB };
			Renderer::GetShaderLibrary().Get("PBR-Deffered-Shader").get()->Bind(ShaderType::Vertex);
			Renderer::GetShaderLibrary().Get("PBR-Deffered-Shader").get()->Bind(ShaderType::Pixel);
			RendererBase::DrawIndexed(buffer);
			Renderer::whiteTexture.Bind(3);
			Renderer::whiteTexture.Bind(4);
			Renderer::whiteTexture.Bind(5);
			Renderer::whiteTexture.Bind(6);
			dst.CopyInto(src);
			//2D Rendering
			Renderer2D::BeginScene(*mainCamera, cameraTransform);
			{
				auto group = m_Registry.group<TransformComponent>(entt::get<SpriteRendererComponent>);
				for (auto& entity : group)
				{
					auto [transform, sprite] = group.get<TransformComponent, SpriteRendererComponent>(entity);
					const auto& transformMatrix = transform.GetTransform({ (entt::entity)m_Registry.get<ParentComponent>(entity).parentID, this });
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
	*/
	void Scene::OnViewportResize(unsigned int width, unsigned int height)
	{
		PT_PROFILE_FUNCTION();
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
	void Scene::UpdateObjectCBs()
	{
		PT_PROFILE_FUNCTION();
		auto view = m_Registry.view<TransformComponent>();
		for (auto entity : view)
		{
			auto& transform = view.get<TransformComponent>(entity);
			if (transform.bDirty)
			{
				TransformData td;
				td.transform = DirectX::XMMatrixTranspose(transform.worldSpaceTransform);
				td.normal = DirectX::XMMatrixInverse(nullptr, transform.worldSpaceTransform);
				if (m_Registry.any_of<MeshRendererComponent>(entity))
				{
					auto& mesh = m_Registry.get<MeshRendererComponent>(entity);
					Renderer::TransformationBuffer[mesh.cbIndex].Update(&td, sizeof(TransformData));
				}
			}
		}
	}
	void Scene::SortMeshComponents()
	{
		bool dirtyMats = false;
		auto view = m_Registry.view<MeshRendererComponent>();
		for (auto entity : view)
		{
			auto& mesh = view.get<MeshRendererComponent>(entity);
			if (mesh.bMaterialDirty)
			{
				mesh.bMaterialDirty = false;
				dirtyMats = true;
			}
		}
		if (dirtyMats)
		{
			PT_CORE_WARN("Actually sorted");
			m_Registry.sort<MeshRendererComponent>([](const MeshRendererComponent& lhs, const MeshRendererComponent& rhs) {return lhs.material.m_uuid < rhs.material.m_uuid; });
		}
	}
	template <typename T>
	void Scene::RenderSpotLightShadows(T& transformMesh, ShadowCastingLight& light)
	{
		RendererBase::ChangeViewport(light.shadowMap.size.x, light.shadowMap.size.y, light.shadowMap.offset.x, light.shadowMap.offset.y);

		//todo make a new constant buffer for shadow code stuff
		Renderer::passConstants.lightSpaceMatrix[0] = DirectX::XMLoadFloat4x4(&light.projection[0]);
		Renderer::passConstants.lightSpaceMatrix[1] = DirectX::XMLoadFloat4x4(&light.projection[0]);
		Renderer::passConstants.lightSpaceMatrix[2] = DirectX::XMLoadFloat4x4(&light.projection[0]);
		Renderer::passConstants.lightSpaceMatrix[3] = DirectX::XMLoadFloat4x4(&light.projection[0]);
		Renderer::UpdatePassConstants();
		for (auto e : transformMesh)
		{
			auto [transform, mesh] = transformMesh.get(e);
			if (mesh.Model.m_uuid)
			{
				Shader::SetVSBuffer(Renderer::TransformationBuffer[mesh.cbIndex], 1);
				Model* model = GetAssetManager()->GetModelResource(mesh.Model);
				Mesh _mesh = model->meshes[mesh.modelIndex];
				Buffer buffer = { &_mesh.GetVertexBuffer(), &_mesh.GetIndexBuffer() };
				RendererBase::DrawIndexed(buffer);
			}
		}
	}
	//todo implement these
	template <typename T>
	void Scene::RenderPointLightShadows(T& transformMesh, ShadowCastingLight& light)
	{

	}
	template <typename T>
	void Scene::RenderDirectionalLightShadows(T& transformMesh, ShadowCastingLight& light)
	{
		ChangeVP(light.shadowMap.size.x / 2, light.shadowMap.offset);
		RendererBase::Getd3dDeviceContext()->RSSetViewports(4, vp);

		//todo make a new constant buffer for shadow code stuff
		Renderer::passConstants.lightSpaceMatrix[0] = DirectX::XMLoadFloat4x4(&light.projection[0]);
		Renderer::passConstants.lightSpaceMatrix[1] = DirectX::XMLoadFloat4x4(&light.projection[1]);
		Renderer::passConstants.lightSpaceMatrix[2] = DirectX::XMLoadFloat4x4(&light.projection[2]);
		Renderer::passConstants.lightSpaceMatrix[3] = DirectX::XMLoadFloat4x4(&light.projection[3]);
		Renderer::UpdatePassConstants();
		for (auto e : transformMesh)
		{
			auto [transform, mesh] = transformMesh.get(e);
			if (mesh.Model.m_uuid)
			{
				Shader::SetVSBuffer(Renderer::TransformationBuffer[mesh.cbIndex], 1);
				Model* model = GetAssetManager()->GetModelResource(mesh.Model);
				Mesh _mesh = model->meshes[mesh.modelIndex];
				Buffer buffer = { &_mesh.GetVertexBuffer(), &_mesh.GetIndexBuffer() };
				RendererBase::DrawIndexed(buffer);
			}
		}

	}
	template<typename T>
	void OnComponentAdded(Entity entity, T& component)
	{
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
		ConstantBuffer cb;
		cb.Create(nullptr, sizeof(TransformData));
		Renderer::TransformationBuffer.push_back(cb);
		component.cbIndex = Renderer::TransformationBuffer.size() - 1;
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
	template<>
	void Scene::OnComponentAdded<CapsuleColliderComponent>(Entity entity, CapsuleColliderComponent& component)
	{
	}
	template<>
	void Scene::OnComponentAdded<PlaneColliderComponent>(Entity entity, PlaneColliderComponent& component)
	{
	}
	template<>
	void Scene::OnComponentAdded<ParentComponent>(Entity entity, ParentComponent& component)
	{
	}

}