#include "ptpch.h"
#include "Material.h"
#include "../Renderer/Renderer.h"
#include "yaml-cpp\yaml.h"
#include "../Asset/AssetManager.h"
namespace YAML {
	template<>
	struct convert<DirectX::XMVECTOR>
	{
		static Node encode(const DirectX::XMVECTOR& v)
		{
			Node node;
			node.push_back(DirectX::XMVectorGetX(v));
			node.push_back(DirectX::XMVectorGetY(v));
			node.push_back(DirectX::XMVectorGetZ(v));
			node.push_back(DirectX::XMVectorGetW(v));
		}

		static bool decode(const Node& node, DirectX::XMVECTOR& v)
		{
			if (!node.IsSequence() || node.size() != 4)
				return false;
			v = DirectX::XMVectorSet(node[0].as<float>(), node[1].as<float>(), node[2].as<float>(), node[3].as<float>());
			return true;
		}

	};
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
namespace Pistachio
{
	YAML::Emitter& operator<<(YAML::Emitter& out, const DirectX::XMFLOAT3& v);
	YAML::Emitter& operator<<(YAML::Emitter& out, const DirectX::XMFLOAT4& v);
	void MaterialSerializer::Serialize(const std::string& filepath, const Material& mat)
	{
		YAML::Emitter out;
		out << YAML::BeginMap;
		out << YAML::Comment("Pistachio Material File");
		out << YAML::Key << "Material" << YAML::Value << "Unnamed Materail";
		out << YAML::Key << "Num Textures" << YAML::Value << (uint32_t)mat.m_textures.size();
		for (uint32_t i = 0; i < mat.m_textures.size();i++)
		{
			out << YAML::Key << i << YAML::Value << GetAssetManager()->GetAssetFileName(mat.m_textures[i]);
		}
		out << YAML::EndMap;

		std::ofstream fout(filepath, std::ios::binary | std::ios::out);
		fout.write(out.c_str(), out.size());
		fout.close();
	}
	
	Material* Material::Create(const char* filepath)
	{
		Material* mat = new Material;
		std::ifstream stream(filepath);
		std::stringstream strStream;
		strStream << stream.rdbuf();
		YAML::Node data = YAML::Load(strStream.str());
		if (!data["Material"])
		{
			PT_CORE_ERROR("The file {0} is not a valid Pistachio Material", filepath);
			return mat;
		}
		uint32_t numTextures = data["Num Textures"].as<uint32_t>();
		for (uint32_t i = 0; i < numTextures; i++)
		{
			mat->m_textures.push_back(GetAssetManager()->CreateTexture2DAsset(data[std::to_string(i)].as<std::string>()));
		}
		std::string shader_name = data["Shader Asset"].as<std::string>();
		mat->shader = GetAssetManager()->CreateShaderAsset(shader_name);
		ShaderAsset* shader = GetAssetManager()->GetShaderResource(mat->shader);
		shader->GetShader()->GetPSShaderBinding(mat->mtlInfo, 3);
		Renderer::AllocateConstantBuffer(shader->GetParamBufferSize());
		
		return mat;
	}
}
