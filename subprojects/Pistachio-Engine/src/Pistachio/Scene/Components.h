#pragma once
#include <DirectXMath.h>
#include <vector>
#include "Pistachio/Core/Math.h"
#include "Pistachio/Renderer/Camera.h"
#include "Pistachio/Renderer/Mesh.h"
#include "Pistachio/Renderer/ShadowMap.h"
#include "Pistachio/Asset/AssetManager.h"
#include "Pistachio/Allocators/AtlasAllocator.h"
#include "Pistachio/Core/Property.h"
#include "Entity.h"
#include "Pistachio/Scene/Scene.h"
namespace Pistachio{
	// Generic ----------------------------------------------------------------------------------
	struct PISTACHIO_API IDComponent
	{
		UUID uuid;
		IDComponent() = default;
		IDComponent(const IDComponent&) = default;
		IDComponent(UUID uuid) : uuid(uuid)
		{}
	};
	struct PISTACHIO_API HierarchyComponent {
		entt::entity parentID = entt::null;
		std::vector<entt::entity> chilren;
	};
	struct PISTACHIO_API TagComponent {
		std::string Tag;
		TagComponent() = default;
		TagComponent(const TagComponent&) = default;
		TagComponent(const std::string& tag) : Tag(tag)
		{}

	};

	struct PISTACHIO_API TransformComponent {
	private:
	public:
		Vector3 Translation = Vector3{ 0,0,0 };
		Quaternion Rotation = Quaternion{ 0,0,0,0 };
		Vector3 Scale = Vector3{ 1,1,1 };
		//Editor Only
		Vector3 RotationEulerHint = Vector3{ 0,0,0 };
		mutable int NumNegativeScaleComps = 0;
		TransformComponent() = default;
		TransformComponent(const TransformComponent&) = default;
		//Editor Only
		DirectX::XMMATRIX worldSpaceTransform = DirectX::XMMatrixIdentity();
		void RecalculateRotation()
		{
			Rotation = Quaternion::CreateFromYawPitchRoll(RotationEulerHint);
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
	struct PISTACHIO_API MeshRendererComponent {
		Asset Model;
		Asset material;
		int modelIndex = 0;
		bool bMaterialDirty = true;
		RendererCBHandle handle;
		~MeshRendererComponent() = default;
		MeshRendererComponent() = default;
		MeshRendererComponent(const MeshRendererComponent& other) = default;
		MeshRendererComponent(const char* path) { Model = GetAssetManager()->CreateModelAsset(path); }
	};
	// ----------------------------------------------------------------------------------------------------------------------
	struct PISTACHIO_API SpriteRendererComponent {
		DirectX::XMFLOAT4 Color = {1.f,1.f,1.f, 1.f}; 
		SpriteRendererComponent() = default;
		SpriteRendererComponent(const SpriteRendererComponent&) = default;
		SpriteRendererComponent(const DirectX::XMFLOAT4& color) : Color(color) {}
		SpriteRendererComponent(const DirectX::XMVECTOR& color){DirectX::XMStoreFloat4(&Color, color); }
	};
	struct PISTACHIO_API CameraComponent {
		SceneCamera camera;
		bool Primary = true;
		bool FixedAspectRatio = false;
		CameraComponent() = default;
		CameraComponent(const CameraComponent&) = default;
	};
	class PISTACHIO_API ScriptableComponent;
	struct PISTACHIO_API NativeScriptComponent {
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
	struct PISTACHIO_API LightComponent {
		LightType Type;
		float Intensity = 1.f;
		DirectX::XMFLOAT3 color = {1,1,1};
		bool shadow = false;
		//for spot lights this holds the outercone and innercone angles(x,y) and the distance for both point and spot light (z)
		DirectX::XMFLOAT3 exData = { 0.01f,0.1f,1.0f };
		DirectX::XMFLOAT3 rotation = {};
		uint32_t shadowMap;
		LightComponent(const LightComponent& other)
		{
			Type = other.Type;
			Intensity = other.Intensity;
			color = other.color;
			shadow = other.shadow;
			exData = other.exData;
			rotation = other.rotation;
		};
		LightComponent() = default;
	};
	// Physics---------------------------------------------------------------------------------------------------------------
	struct PISTACHIO_API RigidBodyComponent {
		enum class BodyType{Static, Dynamic, Kinematic};
		BodyType type = BodyType::Static;
		float Density = 10.f;
		void* RuntimeBody = nullptr;
		float StaticFriction = .5f;
		float DynamicFriction = .5f;
		float Restitution = .5f;
	};
	struct PISTACHIO_API BoxColliderComponent {
		DirectX::XMFLOAT3 size = { 1.f, 1.f, 1.f };
		DirectX::XMFLOAT3 offset = { .0f, .0f, .0f };
		
	};
	struct PISTACHIO_API SphereColliderComponent {
		float size = 1.f;
		DirectX::XMFLOAT3 offset = { .0f, .0f, .0f };
	};
	struct PISTACHIO_API CapsuleColliderComponent {
		float radius = 1.f;
		float height;
		DirectX::XMFLOAT3 offset = { .0f, .0f, .0f };
	};
	struct PISTACHIO_API PlaneColliderComponent {
		float size = 1.f;
		DirectX::XMFLOAT3 offset = { .0f, .0f, .0f };
	};
	// -----------------------------------------------------------------------------------------------------------------------
}
