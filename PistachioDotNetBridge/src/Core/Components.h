#pragma once
#define PT_CUSTOM_ENTRY
#include "Pistachio.h"
#include "ManagedBase.h"
#include "UUID.h"
#include "Math.h"
#include "Asset.h"
namespace PistachioCS
{
	// Generic ----------------------------------------------------------------------------------
	public ref struct IDComponent
	{
	internal:
		Pistachio::IDComponent* m_ptr;
		IDComponent(Pistachio::IDComponent* ref): m_ptr(ref) {}
	public:
		IDComponent(){}
		property PistachioCS::UUID^ UUID
		{
			PistachioCS::UUID^ get()
			{
				return gcnew PistachioCS::UUID(m_ptr->uuid);
			}
		}
	};
	public ref struct ParentComponent {
	internal:
		Pistachio::ParentComponent* m_ptr;
		ParentComponent(Pistachio::ParentComponent* ptr) : m_ptr(ptr) {}
	public:
		ParentComponent(){}
		property System::Int64^ parentID
		{
			System::Int64^ get()
			{
				return gcnew System::Int64(m_ptr->parentID);
			}
		}
	};
	public ref struct TagComponent {
	internal:
		Pistachio::TagComponent* m_ptr;
		TagComponent(Pistachio::TagComponent* ptr) : m_ptr(ptr) {}
	public:
		TagComponent() {}
		property System::String^ Tag
		{
			System::String^ get()
			{
				return gcnew System::String(m_ptr->Tag.c_str());
			}
			void set(System::String^ value)
			{
				m_ptr->Tag = (char*)System::Runtime::InteropServices::Marshal::StringToHGlobalAnsi(value).ToPointer();
			}
		}

	};
	
	public ref struct TransformComponent {
	internal:
		Pistachio::TransformComponent* m_ptr;
		TransformComponent(Pistachio::TransformComponent* ptr) : m_ptr(ptr) {}
	public:
		TransformComponent() {}
		property Vector3^ Translation
		{
			Vector3^ get()
			{
				return gcnew Vector3(m_ptr->Translation.x, m_ptr->Translation.y, m_ptr->Translation.z);
			}
			void set(Vector3^ value)
			{
				m_ptr->Translation = Pistachio::Vector3(value->x, value->y, value->z);
			}
		}
		property Vector3^ Rotation
		{
			Vector3^ get()
			{
				return gcnew Vector3(m_ptr->RotationEulerHint.x, m_ptr->RotationEulerHint.y, m_ptr->RotationEulerHint.z);
			}
			void set(Vector3^ value)
			{
				m_ptr->RotationEulerHint = Pistachio::Vector3(value->x, value->y, value->z);
			}
		}
		property Vector3^ Scale
		{
			Vector3^ get()
			{
				return gcnew Vector3(m_ptr->Scale.x, m_ptr->Scale.y, m_ptr->Scale.z);
			}
			void set(Vector3^ value)
			{
				m_ptr->Scale = Pistachio::Vector3(value->x, value->y, value->z);
			}
		}
	};
	//-----------------------------------------------------------------------------------------------------------------------
	
	// 3D -------------------------------------------------------------------------------------------------------------------
	public ref struct MeshRendererComponent {
	internal:
		Pistachio::MeshRendererComponent* m_ptr;
		MeshRendererComponent(Pistachio::MeshRendererComponent* ptr) : m_ptr(ptr) {}
	public:
		MeshRendererComponent() {}
		property Asset^ Model
		{
			Asset^ get()
			{
				return gcnew Asset(&m_ptr->Model);
			}
			void set(Asset^ value)
			{
				m_ptr->Model = *value->m_ptr;
			}
		}
		property Asset^ Material
		{
			Asset^ get()
			{
				return gcnew Asset(&m_ptr->material);
			}
			void set(Asset^ value)
			{
				m_ptr->Model = *value->m_ptr;
				//todo automatic dirty
				m_ptr->bMaterialDirty = true;
			}
		}
		property System::Int32 ModelIndex
		{
			System::Int32 get()
			{
				return m_ptr->modelIndex;
			}
			void set(System::Int32 value)
			{
				m_ptr->modelIndex = value;
			}
		}
	};
	/*
	// ----------------------------------------------------------------------------------------------------------------------
	struct PISTACHIO_API SpriteRendererComponent {
		DirectX::XMFLOAT4 Color = { 1.f,1.f,1.f, 1.f };
		SpriteRendererComponent() = default;
		SpriteRendererComponent(const SpriteRendererComponent&) = default;
		SpriteRendererComponent(const DirectX::XMFLOAT4& color) : Color(color) {}
		SpriteRendererComponent(const DirectX::XMVECTOR& color) { DirectX::XMStoreFloat4(&Color, color); }
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
		DirectX::XMFLOAT3 color = { 1,1,1 };
		bool shadow = false;
		bool shadow_dirty = true; // todo: mix it in with the other bool on top
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
		enum class BodyType { Static, Dynamic, Kinematic };
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
	*/
}

