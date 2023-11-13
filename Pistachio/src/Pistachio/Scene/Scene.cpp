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
#include "Pistachio/Physics/Physics.h"
#include "Pistachio/Renderer/MeshFactory.h"
static Pistachio::Mesh* ScreenSpaceQuad;
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
static void ChangeVP(std::uint32_t size)
{
	vp[0].Width = vp[0].Height = size;
	vp[1] = vp[2] = vp[3] = vp[0];
	vp[1].TopLeftX = size;
	vp[2].TopLeftY = size;
	vp[3].TopLeftX = vp[3].TopLeftY = size;
}
namespace Pistachio {
	Scene::Scene(SceneDesc desc)
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
		rtDesc.Attachments = { TextureFormat::RGBA8U,  TextureFormat::RGBA16F,TextureFormat::RGBA8U, TextureFormat::RGBA16F,TextureFormat::INT,TextureFormat::D24S8 };
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
		entity.AddComponent<ParentComponent>();
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
		m_Registry.destroy(entity);
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
		PT_PROFILE_FUNCTION()
		UpdateObjectCBs();
		float color[4] = { 0,0,0,0 };
		float color1[4] = { -1,-1,-1,-1 };
		m_gBuffer.ClearAll(color);
		m_gBuffer.Clear(color1, 4);
		m_finalRender.Clear(color, 0);
		m_gBuffer.Bind(0, 5);
		{
			PT_PROFILE_SCOPE("Shadow Rendereing and Light Formation")
			Renderer::whiteTexture.Bind(9);
			Renderer::whiteTexture.Bind(10);
			Renderer::whiteTexture.Bind(11);
			Renderer::whiteTexture.Bind(12);
			auto group = m_Registry.view<TransformComponent, LightComponent>();
			for (auto& entity : group)
			{
				Light light;
				auto [tc, lightcomponent] = group.get<TransformComponent, LightComponent>(entity);
				DirectX::XMStoreFloat4(&light.positionxtype, tc.Translation);
				light.positionxtype.w = (float)lightcomponent.Type;
				DirectX::XMVECTOR lightTransform = DirectX::XMVector3Rotate(DirectX::XMVectorSet(0.f, 0.f, -1.f, 1.f), tc.Rotation);
				DirectX::XMStoreFloat4(&light.rotation, lightTransform);
				light.exData = { lightcomponent.exData.x , lightcomponent.exData.y, lightcomponent.exData.z, (float)lightcomponent.CastShadow};
				light.colorxintensity = { lightcomponent.color.x, lightcomponent.color.y, lightcomponent.color.z, lightcomponent.Intensity };
				DirectX::XMMATRIX lightMatrix[4] = { DirectX::XMMatrixIdentity(),DirectX::XMMatrixIdentity(),DirectX::XMMatrixIdentity(),DirectX::XMMatrixIdentity() };
				if (lightcomponent.CastShadow)
				{
					RendererBase::EnableShadowMapRasetrizerState();
					if(lightcomponent.Type == 0)
					{
						lightMatrix[0] = GetLightMatrixFromCamera(camera.GetViewMatrix(), DirectX::XMMatrixPerspectiveFovLH(DirectX::XMConvertToRadians(camera.GetFOVdeg()), camera.GetAspectRatio(), camera.GetNearClip(), 30.f), light, 1.05f);
						lightMatrix[1] = GetLightMatrixFromCamera(camera.GetViewMatrix(), DirectX::XMMatrixPerspectiveFovLH(DirectX::XMConvertToRadians(camera.GetFOVdeg()), camera.GetAspectRatio(), 30.f, 100.f), light, 1.2f);
						lightMatrix[2] = GetLightMatrixFromCamera(camera.GetViewMatrix(), DirectX::XMMatrixPerspectiveFovLH(DirectX::XMConvertToRadians(camera.GetFOVdeg()), camera.GetAspectRatio(), 100.f, 500.f), light, 1.2f);
						lightMatrix[3] = GetLightMatrixFromCamera(camera.GetViewMatrix(), DirectX::XMMatrixPerspectiveFovLH(DirectX::XMConvertToRadians(camera.GetFOVdeg()), camera.GetAspectRatio(), 500.f, camera.GetFarClip()), light, 1.2f);
					}
					else if (lightcomponent.Type == 2)
					{
						lightMatrix[0] = lightMatrix[1] = lightMatrix[2] = lightMatrix[3] = DirectX::XMMatrixTranspose(DirectX::XMMatrixLookAtLH(tc.Translation, DirectX::XMVectorAdd(tc.Translation, DirectX::XMLoadFloat4(&light.rotation)), DirectX::XMVectorSet(0,1,0,0)) * DirectX::XMMatrixPerspectiveFovLH(DirectX::XMScalarACos(lightcomponent.exData.x) * 2, 1, 0.1f, lightcomponent.exData.z));
					}
					lightcomponent.shadowMap.Clear();
					lightcomponent.shadowMap.Bind();
					auto group = m_Registry.view<TransformComponent, MeshRendererComponent>();
					RendererBase::Getd3dDeviceContext()->PSSetShader(nullptr, nullptr, 0);
					Renderer::GetShaderLibrary().Get("Shadow-Shader")->Bind(ShaderType::Vertex);
					Renderer::GetShaderLibrary().Get("Shadow-Shader")->Bind(ShaderType::Geometry);
					Renderer::passConstants.lightSpaceMatrix[((int)Renderer::passConstants.numlights.x*4)+0] = lightMatrix[0];
					Renderer::passConstants.lightSpaceMatrix[((int)Renderer::passConstants.numlights.x*4)+1] = lightMatrix[1];
					Renderer::passConstants.lightSpaceMatrix[((int)Renderer::passConstants.numlights.x*4)+2] = lightMatrix[2];
					Renderer::passConstants.lightSpaceMatrix[((int)Renderer::passConstants.numlights.x*4)+3] = lightMatrix[3];
					Renderer::passConstants.EyePosW.w = lightcomponent.shadowMap.GetSize();
					ChangeVP(lightcomponent.shadowMap.GetSize()/2);
					RendererBase::Getd3dDeviceContext()->RSSetViewports(4, vp);
					Renderer::UpdatePassConstants();
					for (auto& entity : group)
					{
						//TO-DO MATERIALS
						auto [transform, mesh] = group.get<TransformComponent, MeshRendererComponent>(entity);
						auto model = GetAssetManager()->GetModelResource(mesh.Model);
						if (model) {
							Buffer buffer = { &model->meshes[mesh.modelIndex].GetVertexBuffer(),&model->meshes[mesh.modelIndex].GetIndexBuffer()};
							Shader::SetVSBuffer(Renderer::TransformationBuffer[mesh.cbIndex], 1);

							RendererBase::DrawIndexed(buffer);
						}
					}
					RendererBase::ChangeViewport(m_gBuffer.GetWidth(), m_gBuffer.GetHeight());
					m_gBuffer.Bind(0, 5);
					lightcomponent.shadowMap.BindResource(9 + Renderer::passConstants.numlights.x);
					RendererBase::SetCullMode(CullMode::Back);
				}
				Renderer::AddLight(light);
				RendererBase::Getd3dDeviceContext()->GSSetShader(nullptr, nullptr, 0);
			}
		}
		{
			PT_PROFILE_SCOPE("Object Rendering (Gbuffer Write)")
			Renderer::BeginScene(camera);
			auto group = m_Registry.view<TransformComponent, MeshRendererComponent>();
			for (auto& entity : group)
			{	
				//TO-DO MATERIALS
				auto [transform, mesh] = group.get<TransformComponent, MeshRendererComponent>(entity);
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
				Renderer2D::DrawSprite(transformMatrix, sprite, (int)entity);
			}
		}
		Renderer2D::EndScene();
	}
	void Scene::OnUpdateRuntime(float delta)
	{
		PT_PROFILE_FUNCTION();
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
			float color[4] = { 0,0,0,0 };
			float color1[4] = { -1,-1,-1,-1 };
			m_gBuffer.Clear(color, 0);
			m_gBuffer.Clear(color, 1);
			m_gBuffer.Clear(color, 2);
			m_gBuffer.Clear(color, 3);
			m_gBuffer.Clear(color1, 4);
			m_finalRender.Clear(color, 0);
			m_gBuffer.Bind(0, 5);
			{
				DirectX::XMMATRIX view = DirectX::XMMatrixInverse(nullptr, cameraTransform);
				Renderer::whiteTexture.Bind(9);
				Renderer::whiteTexture.Bind(10);
				Renderer::whiteTexture.Bind(11);
				Renderer::whiteTexture.Bind(12);
				auto group = m_Registry.view<TransformComponent, LightComponent>();
				for (auto& entity : group)
				{
					Light light;
					auto [tc, lightcomponent] = group.get<TransformComponent, LightComponent>(entity);
					DirectX::XMStoreFloat4(&light.positionxtype, tc.Translation);
					light.positionxtype.w = (float)lightcomponent.Type;
					DirectX::XMVECTOR lightTransform = DirectX::XMVector3Rotate(DirectX::XMVectorSet(0.f, 0.f, -1.f, 1.f), tc.Rotation);
					DirectX::XMStoreFloat4(&light.rotation, lightTransform);
					light.exData = { lightcomponent.exData.x , lightcomponent.exData.y, lightcomponent.exData.z, (float)lightcomponent.CastShadow };
					light.colorxintensity = { lightcomponent.color.x, lightcomponent.color.y, lightcomponent.color.z, lightcomponent.Intensity };
					DirectX::XMMATRIX lightMatrix[4] = { DirectX::XMMatrixIdentity(),DirectX::XMMatrixIdentity(),DirectX::XMMatrixIdentity(),DirectX::XMMatrixIdentity() };
					if (lightcomponent.CastShadow)
					{
						RendererBase::SetCullMode(CullMode::Front);
						if (lightcomponent.Type == 0)
						{
							float aspect = (float)m_viewportWidth / (float)m_ViewportHeight;
							lightMatrix[0] = GetLightMatrixFromCamera(view, DirectX::XMMatrixPerspectiveFovLH(mainCamera->GetPerspSize(), aspect, mainCamera->GetPerspNear(), 10.f), light, 1.05f);
							lightMatrix[1] = GetLightMatrixFromCamera(view, DirectX::XMMatrixPerspectiveFovLH(mainCamera->GetPerspSize(), aspect, 10.f, 50.f), light, 1.2f);
							lightMatrix[2] = GetLightMatrixFromCamera(view, DirectX::XMMatrixPerspectiveFovLH(mainCamera->GetPerspSize(), aspect, 50.f, 70.f), light, 1.f);
							lightMatrix[3] = GetLightMatrixFromCamera(view, DirectX::XMMatrixPerspectiveFovLH(mainCamera->GetPerspSize(), aspect, 70.f, mainCamera->GetPerspFar()), light, 1.f);
						}
						else if (lightcomponent.Type == 2)
						{
							lightMatrix[0] = lightMatrix[1] = lightMatrix[2] = lightMatrix[3] = DirectX::XMMatrixTranspose(DirectX::XMMatrixLookAtLH(tc.Translation, DirectX::XMVectorAdd(tc.Translation, DirectX::XMLoadFloat4(&light.rotation)), DirectX::XMVectorSet(0, 1, 0, 0)) * DirectX::XMMatrixPerspectiveFovLH(DirectX::XMScalarACos(lightcomponent.exData.x) * 2, 1, 0.1f, lightcomponent.exData.z));
						}
						lightcomponent.shadowMap.Clear();
						lightcomponent.shadowMap.Bind();
						auto group = m_Registry.view<TransformComponent, MeshRendererComponent>();
						RendererBase::Getd3dDeviceContext()->PSSetShader(nullptr, nullptr, 0);
						Renderer::GetShaderLibrary().Get("Shadow-Shader")->Bind(ShaderType::Vertex);
						Renderer::GetShaderLibrary().Get("Shadow-Shader")->Bind(ShaderType::Geometry);
						Renderer::passConstants.lightSpaceMatrix[((int)Renderer::passConstants.numlights.x * 4) + 0] = lightMatrix[0];
						Renderer::passConstants.lightSpaceMatrix[((int)Renderer::passConstants.numlights.x * 4) + 1] = lightMatrix[1];
						Renderer::passConstants.lightSpaceMatrix[((int)Renderer::passConstants.numlights.x * 4) + 2] = lightMatrix[2];
						Renderer::passConstants.lightSpaceMatrix[((int)Renderer::passConstants.numlights.x * 4) + 3] = lightMatrix[3];
						RendererBase::Getd3dDeviceContext()->RSSetViewports(4, vp);
						for (auto& entity : group)
						{
							//TO-DO MATERIALS
							auto [transform, mesh] = group.get<TransformComponent, MeshRendererComponent>(entity);
							auto model = GetAssetManager()->GetModelResource(mesh.Model);
							if (model) {
								auto& VB = model->meshes[mesh.modelIndex].GetVertexBuffer();
								auto& IB = model->meshes[mesh.modelIndex].GetIndexBuffer();
								Buffer buffer = { &VB,&IB };
								RendererBase::DrawIndexed(buffer);
							}
						}
						RendererBase::ChangeViewport(m_gBuffer.GetWidth(), m_gBuffer.GetHeight());
						m_gBuffer.Bind(0, 5);
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
				auto group = m_Registry.view<TransformComponent, MeshRendererComponent>();
				for (auto& entity : group)
				{
					//TO-DO MATERIALS
					auto [transform, mesh] = group.get<TransformComponent, MeshRendererComponent>(entity);
					const auto& transformMatrix = transform.GetTransform({ (entt::entity)m_Registry.get<ParentComponent>(entity).parentID, this });
					auto mat = GetAssetManager()->GetMaterialResource(mesh.material);
					auto model= GetAssetManager()->GetModelResource(mesh.Model);
					float c[4]{
							std::pow(mat->diffuseColor.x, 2.2),
							std::pow(mat->diffuseColor.y, 2.2),
							std::pow(mat->diffuseColor.z, 2.2),
							1.0
					};
					if (model)
						Renderer::Submit(&model->meshes[mesh.modelIndex], Renderer::GetShaderLibrary().Get("GBuffer-Shader").get(), mat,(uint32_t)entity);
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
		auto view = m_Registry.view<MeshRendererComponent, TransformComponent>();
		for (auto entity : view)
		{
			auto& transform = view.get<TransformComponent>(entity);
			auto& mesh = view.get<MeshRendererComponent>(entity);
			if (transform.bDirty)
			{
				TransformData td;
				auto transformMat = transform.GetTransform({ (entt::entity)m_Registry.get<ParentComponent>(entity).parentID, this });
				td.transform = DirectX::XMMatrixTranspose(transformMat);
				td.normal = DirectX::XMMatrixInverse(nullptr, transformMat);
				Renderer::TransformationBuffer[mesh.cbIndex].Update(&td, sizeof(TransformData));
				transform.bDirty = false;
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