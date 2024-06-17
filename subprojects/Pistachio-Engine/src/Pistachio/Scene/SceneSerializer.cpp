#include "Pistachio/Scene/Scene.h"
#include "ptpch.h"
#include "SceneSerializer.h"
#include "Entity.h"
#include "Components.h"
#define YAML_CPP_STATIC_DEFINE
#include "yaml-cpp/yaml.h"
#include "../Asset/AssetManager.h"


namespace YAML {
	
	template<>
	struct convert<DirectX::XMFLOAT3>
	{
		static Node encode(const DirectX::XMFLOAT3& v)
		{
			Node node;
			node.push_back(v.x);
			node.push_back(v.y);
			node.push_back(v.z);
		}

		static bool decode(const Node& node, DirectX::XMFLOAT3& v)
		{
			if (!node.IsSequence() || node.size() != 3)
				return false;
			v = { node[0].as<float>(), node[1].as<float>(), node[2].as<float>() };
			return true;
		}

	};
	template<>
	struct convert<DirectX::XMFLOAT4>
	{
		static Node encode(const DirectX::XMFLOAT4& v)
		{
			Node node;
			node.push_back(v.x);
			node.push_back(v.y);
			node.push_back(v.z);
			node.push_back(v.w);
		}

		static bool decode(const Node& node, DirectX::XMFLOAT4& v)
		{
			if (!node.IsSequence() || node.size() != 4)
				return false;
			v = { node[0].as<float>(), node[1].as<float>(), node[2].as<float>(), node[3].as<float>() };
			return true;
		}

	};
	
}

namespace Pistachio {

	
	YAML::Emitter& operator<<(YAML::Emitter& out, const DirectX::XMFLOAT3& v)
	{
		out << YAML::Flow;
		out << YAML::BeginSeq << v.x << v.y << v.z <<  YAML::EndSeq;
		return out;
	}
	YAML::Emitter& operator<<(YAML::Emitter& out, const DirectX::XMFLOAT4& v)
	{
		out << YAML::Flow;
		out << YAML::BeginSeq << v.x << v.y << v.z << v.w << YAML::EndSeq;
		return out;
	}
	SceneSerializer::SceneSerializer(const Ref<Scene>& scene)
		:m_Scene(scene)
	{
	}
	static void SerializeEntity(YAML::Emitter& out, Entity entity, Ref<Scene> scene)
	{
		out << YAML::BeginMap;
		out << YAML::Key << "Entity" << YAML::Value << entity.GetComponent<IDComponent>().uuid;
		if (entity.HasComponent<TagComponent>())
		{
			out << YAML::Key << "TagComponent";
			out << YAML::BeginMap;
			auto& tag = entity.GetComponent<TagComponent>().Tag;
			out << YAML::Key << "Tag" << YAML::Value << tag;
			out << YAML::EndMap;
		}
		if (entity.HasComponent<HierarchyComponent>())
		{
			out << YAML::Key << "ParentComponent";
			out << YAML::BeginMap;
			auto pid = entity.GetComponent<HierarchyComponent>().parentID;
			auto parent = Entity((entt::entity)entity.GetComponent<HierarchyComponent>().parentID, scene.get());
			if(pid != entt::null)
				out << YAML::Key << "ParentID" << YAML::Value << parent.GetComponent<IDComponent>().uuid;
			else
				out << YAML::Key << "ParentID" << YAML::Value << 0;
			out << YAML::EndMap;
		}
		if (entity.HasComponent<TransformComponent>())
		{
			out << YAML::Key << "TransformComponent";
			out << YAML::BeginMap;
			auto& tc = entity.GetComponent<TransformComponent>();
			out << YAML::Key << "Translation" << YAML::Value << tc.Translation;
			out << YAML::Key << "Rotation" << YAML::Value << tc.Rotation;
			out << YAML::Key << "RotationEulerHint" << YAML::Value << tc.RotationEulerHint;
			out << YAML::Key << "Scale" << YAML::Value << tc.Scale;
			out << YAML::EndMap;
		}
		if (entity.HasComponent<MeshRendererComponent>())
		{
			out << YAML::Key << "MeshRendererComponent";
			out << YAML::BeginMap;
			auto& mr = entity.GetComponent<MeshRendererComponent>();
			out << YAML::Key << "Model" << GetAssetManager()->GetAssetFileName(mr.Model);
			out << YAML::Key << "Material" << GetAssetManager()->GetAssetFileName(mr.material);
			out << YAML::Key << "Model Index" << mr.modelIndex;
			out << YAML::EndMap;

		}
		if (entity.HasComponent<SpriteRendererComponent>())
		{
			out << YAML::Key << "SpriteRendererComponent";
			out << YAML::BeginMap;
			auto& src = entity.GetComponent<SpriteRendererComponent>();
			out << YAML::Key << "Color" << YAML::Value << src.Color;
			out << YAML::EndMap;
		}
		if (entity.HasComponent<CameraComponent>())
		{
			out << YAML::Key << "CameraComponent";
			out << YAML::BeginMap;
			auto& cc = entity.GetComponent<CameraComponent>();
			auto& camera = cc.camera;
			out << YAML::Key << "Camera";
			out << YAML::BeginMap;
			out << YAML::Key << "ProjectionType" << YAML::Value << (int)camera.GetProjectionType();
			out << YAML::Key << "PerspectiveFOV" << YAML::Value << camera.GetPerspSize();
			out << YAML::Key << "PerspectiveNear" << YAML::Value << camera.GetPerspNear();
			out << YAML::Key << "PerspectiveFar" << YAML::Value << camera.GetPerspFar();
			out << YAML::Key << "OrthographicSize" << YAML::Value << camera.GetOrthoSize();
			out << YAML::Key << "OrthographicNear" << YAML::Value << camera.GetOrthoNear();
			out << YAML::Key << "OrthographicFar" << YAML::Value << camera.GetOrthoFar();
			out << YAML::EndMap;
			out << YAML::Key << "Primary" << YAML::Value << cc.Primary;
			out << YAML::Key << "FixedAspect" << YAML::Value << cc.FixedAspectRatio;
			out << YAML::EndMap;
		}
		if (entity.HasComponent<LightComponent>())
		{
			auto& lc = entity.GetComponent<LightComponent>();
			out << YAML::Key << "LightComponent";
			out << YAML::BeginMap;
			out << YAML::Key << "Color" << YAML::Value << lc.color;
			out << YAML::Key << "Intensity" << YAML::Value << lc.Intensity;
			out << YAML::Key << "Type" << YAML::Value << (int)lc.Type;
			out << YAML::EndMap;
		}
		if (entity.HasComponent<RigidBodyComponent>())
		{
			auto& rbc = entity.GetComponent<RigidBodyComponent>();
			out << YAML::Key << "RigidBodyComponent";
			out << YAML::BeginMap;
			out << YAML::Key << "BodyType" << YAML::Comment("0 = Static, 1 = Dynamic, 2 = Kinematic")<< YAML::Value << (int)rbc.type;
			out << YAML::Key << "Density" << YAML::Value << rbc.Density;
			out << YAML::Key << "StaticFriction" << YAML::Value << rbc.StaticFriction;
			out << YAML::Key << "DynamicFriction" << YAML::Value << rbc.DynamicFriction;
			out << YAML::Key << "DynamicFriction" << YAML::Value << rbc.Restitution;
			out << YAML::EndMap;
		}
		if (entity.HasComponent<BoxColliderComponent>())
		{
			auto& bc = entity.GetComponent<BoxColliderComponent>();
			out << YAML::Key << "BoxColliderComponent";
			out << YAML::BeginMap;
			out << YAML::Key << "Size" << YAML::Value << bc.size;
			out << YAML::Key << "Offset" << YAML::Value << bc.offset;
			out << YAML::EndMap;
		}

		out << YAML::EndMap;
	}
	void SceneSerializer::Serialize(const std::string& filepath)
	{
		YAML::Emitter out;
		out << YAML::BeginMap;
		out << YAML::Comment("Pistachio Scene File (*.ptscene) Generated by the Serializer");
		out << YAML::Key << "Scene" << YAML::Value << "Unnamed Scene";
		out << YAML::Key << "Entities";
		out << YAML::Value << YAML::BeginSeq;
		m_Scene->m_Registry.each([&](auto entityID) {
			Entity entity = { entityID, m_Scene.get() };
			if (!entity)
				return;
			SerializeEntity(out,entity, m_Scene);
		
		});
		out << YAML::EndSeq;
		out << YAML::EndMap;

		std::ofstream fout(filepath, std::ios::binary | std::ios::out);
		fout.write(out.c_str(), out.size());
		fout.close();
	}
	void SceneSerializer::DeSerialize(const std::string& filepath)
	{
		std::ifstream stream(filepath);
		std::stringstream strStream;
		strStream << stream.rdbuf();

		YAML::Node data = YAML::Load(strStream.str());
		if (!data["Scene"])
		{
			PT_CORE_ERROR("The file {0} is not a valid Pistachio Scene", filepath);
			return;
		}
		m_Scene->m_Registry.clear();
		std::string sceneName = data["Scene"].as<std::string>();
		YAML::Node Entities = data["Entities"];
		if (Entities)
		{
			std::vector<Entity> entities;
			for (auto entity : Entities)
			{
				uint64_t uuid = entity["Entity"].as<uint64_t>();
				std::string name;

				auto tagComponent = entity["TagComponent"];
				if (tagComponent)
					name = tagComponent["Tag"].as<std::string>();
				entities.push_back(m_Scene->CreateEntityWithUUID(uuid,name));
				Entity DeserializedEntity = entities[entities.size() - 1];
				auto transformComponent = entity["TransformComponent"];
				if (transformComponent)
				{
					auto& tc = DeserializedEntity.GetComponent<TransformComponent>();
					tc.Translation = transformComponent["Translation"].as<DirectX::XMFLOAT3>();
					tc.Rotation = transformComponent["Rotation"].as<DirectX::XMFLOAT4>();
					tc.Scale = transformComponent["Scale"].as<DirectX::XMFLOAT3>();
				}

				auto cameraComponent = entity["CameraComponent"];
				if (cameraComponent)
				{
					auto& cc = DeserializedEntity.AddComponent<CameraComponent>();

					const auto& cameraProps = cameraComponent["Camera"];
					if (cameraProps["ProjectionType"].as<int>() == 0)
					{
						cc.camera.SetOrthographic(cameraProps["OrthographicSize"].as<float>(), cameraProps["OrthographicNear"].as<float>(), cameraProps["OrthographicFar"].as<float>());
						cc.camera.SetPerspective(cameraProps["PerspectiveFOV"].as<float>(), cameraProps["PerspectiveNear"].as<float>(), cameraProps["PerspectiveFar"].as<float>());
					}
					else
					{
						cc.camera.SetPerspective(cameraProps["PerspectiveFOV"].as<float>(), cameraProps["PerspectiveNear"].as<float>(), cameraProps["PerspectiveFar"].as<float>());
						cc.camera.SetOrthographic(cameraProps["OrthographicSize"].as<float>(), cameraProps["OrthographicNear"].as<float>(), cameraProps["OrthographicFar"].as<float>());
					}
					cc.Primary = cameraComponent["Primary"].as<bool>();
					cc.FixedAspectRatio = cameraComponent["FixedAspect"].as<bool>();
				}

				auto spriteComponent = entity["SpriteRendererComponent"];
				if (spriteComponent)
				{
					auto& src = DeserializedEntity.AddComponent<SpriteRendererComponent>();
					src.Color = spriteComponent["Color"].as<DirectX::XMFLOAT4>();
				}

				auto lightComponent = entity["LightComponent"];
				if (lightComponent)
				{
					auto& lc = DeserializedEntity.AddComponent<LightComponent>();
					lc.color = lightComponent["Color"].as<DirectX::XMFLOAT3>();
					lc.Intensity = lightComponent["Intensity"].as<float>();
					lc.Type = (LightType)lightComponent["Type"].as<float>();
				}
				auto rigidbodycomponent = entity["RigidBodyComponent"];
				if (rigidbodycomponent)
				{
					auto& rbc = DeserializedEntity.AddComponent<RigidBodyComponent>();
					rbc.Density = rigidbodycomponent["Density"].as<float>();
					rbc.type = (RigidBodyComponent::BodyType)rigidbodycomponent["Type"].as<int>();
					rbc.DynamicFriction = rigidbodycomponent["DynamicFriction"].as<float>();
					rbc.StaticFriction = rigidbodycomponent["StaticFriction"].as<float>();
					rbc.Restitution = rigidbodycomponent["Restitution"].as<float>();
				}
				auto boxcollidercomponent = entity["BoxColliderComponent"];
				if (boxcollidercomponent)
				{
					auto& bc = DeserializedEntity.AddComponent<BoxColliderComponent>();
					bc.size = boxcollidercomponent["Size"].as<DirectX::XMFLOAT3>();
					bc.offset = boxcollidercomponent["Offset"].as<DirectX::XMFLOAT3>();
				}
				auto meshrenderercomponent = entity["MeshRendererComponent"];
				if (meshrenderercomponent)
				{
					auto& mr = DeserializedEntity.AddComponent<MeshRendererComponent>();
					auto mat = meshrenderercomponent["Material"].as<std::string>();
					auto model = meshrenderercomponent["Model"].as<std::string>();
					if (mat != "None")
						mr.material = GetAssetManager()->CreateMaterialAsset(mat);
					if (model != "None")
						mr.Model = GetAssetManager()->CreateModelAsset(model);
				}
			}
			int i = 0;
			for (auto entity : Entities)
			{
				auto parentComponent = entity["ParentComponent"];
				auto pida = parentComponent["ParentID"];
				auto pid = pida.as<std::uint64_t>();
				auto view = m_Scene->m_Registry.view<IDComponent>();
				entities[i].GetComponent<HierarchyComponent>().parentID = entt::null;
				for (auto entt : view)
				{
					auto id = view.get<IDComponent>(entt);
					if (pid == id.uuid)
					{
						entities[i].GetComponent<HierarchyComponent>().parentID = entt;
					}
				}
				i++;
			}
		}
	}
}