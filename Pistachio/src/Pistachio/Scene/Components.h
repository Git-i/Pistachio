#pragma once
#include <DirectXMath.h>
#include "Pistachio/Renderer/Camera.h"
#include "Pistachio/Renderer/Mesh.h"
#include "Pistachio/Core/UUID.h"
namespace Pistachio{
	struct IDComponent
	{
		UUID uuid;
		IDComponent() = default;
		IDComponent(const IDComponent&) = default;
		IDComponent(UUID uuid) : uuid(uuid)
		{}
	};
	struct TagComponent {
		std::string Tag;
		TagComponent() = default;
		TagComponent(const TagComponent&) = default;
		TagComponent(const std::string& tag) : Tag(tag)
		{}

	};
	struct TransformComponent {
		DirectX::XMVECTOR Translation = DirectX::XMVectorZero();
		DirectX::XMVECTOR Rotation = DirectX::XMQuaternionIdentity();
		//Editor Only
		DirectX::XMFLOAT4 RotationEulerHint = {0.f,0.f,0.f,0.f};
		DirectX::XMVECTOR Scale = {1.f,1.f,1.f,1.f};
		mutable int NumNegativeScaleComps = 0;
		TransformComponent() = default;
		TransformComponent(const TransformComponent&) = default;
		TransformComponent(const DirectX::XMVECTOR& translation) : Translation(translation){}
		//Editor Only
		void RecalculateRotation()
		{
			Rotation = DirectX::XMQuaternionRotationRollPitchYawFromVector(DirectX::XMLoadFloat4(&RotationEulerHint));
		}
		DirectX::XMMATRIX GetTransform() const
		{
			NumNegativeScaleComps = 0;
			for (int i = 0; i < 3; i++)
				if (((float*)&Scale)[i] < 0) NumNegativeScaleComps++;
			return DirectX::XMMatrixScalingFromVector(Scale) *  DirectX::XMMatrixRotationQuaternion(Rotation) * DirectX::XMMatrixTranslationFromVector(Translation);
		}
	};
	struct MeshRendererComponent {
		Ref<Pistachio::Mesh> Mesh = nullptr;
		DirectX::XMFLOAT3 color = { 1,1,1 };
		float roughness = 0.5f;
		float metallic = 0.5f;
		MeshRendererComponent() :Mesh(nullptr){ };
		MeshRendererComponent(const MeshRendererComponent&) = default;
		MeshRendererComponent(const char* path) { Mesh.reset(Pistachio::Mesh::Create(path)); }
	};
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
		int Type;
		float Intensity;
		DirectX::XMFLOAT3 color;
	};
	struct RigidBodyComponent {
		enum class BodyType{Static, Dynamic, Kinematic};
		BodyType type = BodyType::Static;
		float Density = 10.f;
		void* RuntimeBody = nullptr;
	};
	struct BoxColliderComponent {
		DirectX::XMFLOAT3 size = { 1.f, 1.f, 1.f };
		DirectX::XMFLOAT3 offset = { .0f, .0f, .0f };
		float StaticFriction = .5f;
		float DynamicFriction = .5f;
		float Restitution = .5f;
	};
	struct SphereColliderComponent {
		DirectX::XMFLOAT3 size = { 1.f, 1.f, 1.f };
		DirectX::XMFLOAT3 offset = { .0f, .0f, .0f };
	};
}
