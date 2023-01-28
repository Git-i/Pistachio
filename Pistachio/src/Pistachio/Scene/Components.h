#pragma once
#include <DirectXMath.h>
#include "Pistachio/Renderer/Camera.h"
#include "Pistachio/Renderer/Mesh.h"
#include "ScriptableComponent.h"
namespace Pistachio{
	struct TagComponent {
		std::string Tag;
		TagComponent() = default;
		TagComponent(const TagComponent&) = default;
		TagComponent(const std::string& tag) : Tag(tag)
		{}

	};
	struct TransformComponent {
		DirectX::XMVECTOR Translation = DirectX::XMVectorZero();
		DirectX::XMVECTOR Rotation = DirectX::XMVectorZero();
		DirectX::XMVECTOR Scale = {1.f,1.f,1.f,1.f};
		mutable int NumNegativeScaleComps = 0;
		TransformComponent() = default;
		TransformComponent(const TransformComponent&) = default;
		TransformComponent(const DirectX::XMVECTOR& translation) : Translation(translation){}
		DirectX::XMMATRIX GetTransform() const
		{
			NumNegativeScaleComps = 0;
			for (int i = 0; i < 3; i++)
				if (((float*)&Scale)[i] < 0) NumNegativeScaleComps++;
			return DirectX::XMMatrixScalingFromVector(Scale) *  DirectX::XMMatrixRotationX(DirectX::XMVectorGetX(Rotation)) * DirectX::XMMatrixRotationY(DirectX::XMVectorGetY(Rotation))* DirectX::XMMatrixRotationZ(DirectX::XMVectorGetZ(Rotation)) * DirectX::XMMatrixTranslationFromVector(Translation);
		}
	};
	struct MeshRendererComponent {
		Pistachio::Mesh Mesh;
		MeshRendererComponent() = default;
		MeshRendererComponent(const MeshRendererComponent&) = default;
		MeshRendererComponent(const Pistachio::Mesh& mesh) : Mesh(mesh){}
		MeshRendererComponent(const char* path) { Mesh.CreateStack(path); }
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
}
