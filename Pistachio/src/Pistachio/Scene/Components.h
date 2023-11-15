#pragma once
#include <DirectXMath.h>
#include "Pistachio/Renderer/Camera.h"
#include "Pistachio/Renderer/Mesh.h"
#include "Pistachio\Renderer\ShadowMap.h"
#include "Pistachio\Asset\AssetManager.h"
#include "Entity.h"
namespace Pistachio{
	// Generic ----------------------------------------------------------------------------------
	struct IDComponent
	{
		UUID uuid;
		IDComponent() = default;
		IDComponent(const IDComponent&) = default;
		IDComponent(UUID uuid) : uuid(uuid)
		{}
	};
	struct ParentComponent {
		std::int64_t parentID = 0;
	};
	struct TagComponent {
		std::string Tag;
		TagComponent() = default;
		TagComponent(const TagComponent&) = default;
		TagComponent(const std::string& tag) : Tag(tag)
		{}

	};
	struct TransformComponent {
		Vector3 Translation = Vector3::Zero;
		Quaternion Rotation = Quaternion::Identity;
		//Editor Only
		Vector3 RotationEulerHint;
		Vector3 Scale = {1.f,1.f,1.f};
		mutable int NumNegativeScaleComps = 0;
		bool bDirty = true;
		
		TransformComponent() = default;
		TransformComponent(const TransformComponent&) = default;
		TransformComponent(const DirectX::XMVECTOR& translation) : Translation(translation){}
		//Editor Only
		void RecalculateRotation()
		{
			Rotation = Quaternion::CreateFromYawPitchRoll(RotationEulerHint);
		}
		DirectX::XMMATRIX GetTransform(Entity parent) const
		{
			NumNegativeScaleComps = 0;
			for (int i = 0; i < 3; i++)
				if (((float*)&Scale)[i] < 0) NumNegativeScaleComps++;
			int PID = parent.GetComponent<ParentComponent>().parentID;
			DirectX::XMMATRIX parentTransform = PID >= 0 ? (parent.GetComponent<TransformComponent>().GetTransform({ (entt::entity)PID, parent.m_Scene})) : parent.GetComponent<TransformComponent>().GetLocalTransform();
			return (DirectX::XMMatrixScalingFromVector(Scale) * DirectX::XMMatrixRotationQuaternion(Rotation) * DirectX::XMMatrixTranslationFromVector(Translation)) * parentTransform;
		}
		DirectX::XMMATRIX GetLocalTransform() const
		{
			NumNegativeScaleComps = 0;
			for (int i = 0; i < 3; i++)
				if (((float*)&Scale)[i] < 0) NumNegativeScaleComps++;
			return (DirectX::XMMatrixScalingFromVector(Scale) * DirectX::XMMatrixRotationQuaternion(Rotation) * DirectX::XMMatrixTranslationFromVector(Translation));
		}
	};
	//-----------------------------------------------------------------------------------------------------------------------

	// 3D -------------------------------------------------------------------------------------------------------------------
	struct MeshRendererComponent {
		Asset Model;
		Asset material;
		std::size_t cbIndex = 0;
		int modelIndex = 0;
		bool bMaterialDirty = true;
		~MeshRendererComponent() = default;
		MeshRendererComponent() = default;
		MeshRendererComponent(const MeshRendererComponent& other) = default;
		MeshRendererComponent(const char* path) { Model = GetAssetManager()->CreateModelAsset(path); }
	};
	// ----------------------------------------------------------------------------------------------------------------------
	struct SpriteRendererComponent {
		DirectX::XMFLOAT4 Color = {1.f,1.f,1.f, 1.f}; 
		SpriteRendererComponent() = default;
		SpriteRendererComponent(const SpriteRendererComponent&) = default;
		SpriteRendererComponent(const DirectX::XMFLOAT4& color) : Color(color) {}
		SpriteRendererComponent(const DirectX::XMVECTOR& color){DirectX::XMStoreFloat4(&Color, color); }
	};
	struct CameraComponent {
		SceneCamera camera;
		bool Primary = true;
		bool FixedAspectRatio = false;
		CameraComponent() = default;
		CameraComponent(const CameraComponent&) = default;
	};
	class ScriptableComponent;
	struct NativeScriptComponent {
		ScriptableComponent* Instance = nullptr;
		
		std::function<void()> InstantiateFunction;
		std::function<void()> DestroyInstanceFunction;
		std::function<void(ScriptableComponent*)> OnCreateFunction;
		std::function<void(ScriptableComponent*)> OnDestroyFunction;
		std::function<void(ScriptableComponent*, float delta)> OnUpdateFunction;
		template<typename T>
		void Bind()
		{
			InstantiateFunction = [&]() { Instance = new T(); };
			DestroyInstanceFunction = [&]() { delete (T*)Instance; };
			OnCreateFunction = [](ScriptableComponent* instance) { ((T*)instance)->OnCreate(); };
			OnDestroyFunction = [](ScriptableComponent* instance) { ((T*)instance)->OnDestroy(); };
			OnUpdateFunction = [](ScriptableComponent* instance, float delta) { ((T*)instance)->OnUpdate(delta); };
		}
	};
	struct LightComponent {
		int Type = 0;
		float Intensity = 1.f;
		DirectX::XMFLOAT3 color = {1,1,1};
		bool CastShadow = false;
		DirectX::XMFLOAT3 exData = { 0.01f,0.1f,1.0f };
		DirectX::XMFLOAT3 rotation;
		ShadowMap shadowMap;
		LightComponent(const LightComponent& other)
		{
			Type = other.Type;
			Intensity = other.Intensity;
			color = other.color;
			CastShadow = other.CastShadow;
			exData = other.exData;
			rotation = other.rotation;
		}
		LightComponent() = default;
	};
	// Physics---------------------------------------------------------------------------------------------------------------
	struct RigidBodyComponent {
		enum class BodyType{Static, Dynamic, Kinematic};
		BodyType type = BodyType::Static;
		float Density = 10.f;
		void* RuntimeBody = nullptr;
		float StaticFriction = .5f;
		float DynamicFriction = .5f;
		float Restitution = .5f;
	};
	struct BoxColliderComponent {
		DirectX::XMFLOAT3 size = { 1.f, 1.f, 1.f };
		DirectX::XMFLOAT3 offset = { .0f, .0f, .0f };
		
	};
	struct SphereColliderComponent {
		float size = 1.f;
		DirectX::XMFLOAT3 offset = { .0f, .0f, .0f };
	};
	struct CapsuleColliderComponent {
		float radius = 1.f;
		float height;
		DirectX::XMFLOAT3 offset = { .0f, .0f, .0f };
	};
	struct PlaneColliderComponent {
		float size = 1.f;
		DirectX::XMFLOAT3 offset = { .0f, .0f, .0f };
	};
	// -----------------------------------------------------------------------------------------------------------------------
}
