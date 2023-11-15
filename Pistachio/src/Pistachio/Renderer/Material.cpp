#include "ptpch.h"
#include "Material.h"
#include "yaml-cpp\yaml.h"
#include "../Asset/AssetManager.h"
#include "../Renderer/Renderer.h"
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
		out << YAML::Key << "DiffuseColor" << YAML::Value << (DirectX::XMFLOAT4)mat.diffuseColor;
		out << YAML::Key << "Metallic" << YAML::Value << mat.metallic;
		out << YAML::Key << "Roughness" << YAML::Value << mat.roughness;
		out << YAML::Key << "DiffuseTex" << YAML::Value << mat.diffuseTexName;
		out << YAML::Key << "NormalTex" << YAML::Value << mat.normalTexName;
		out << YAML::Key << "MetallicTex" << YAML::Value << mat.metallicTexName;
		out << YAML::Key << "RoughnessTex" << YAML::Value << mat.roughnessTexName;
		out << YAML::EndMap;

		std::ofstream fout(filepath, std::ios::binary | std::ios::out);
		fout.write(out.c_str(), out.size());
		fout.close();
	}
	void Material::Initialize()
	{
		data.Create(nullptr, sizeof(MaterialStruct));
	}
	void Material::Bind()
	{
		Shader::SetPSBuffer(data, 1);
		auto diff = GetAssetManager()->GetTexture2DResource(diffuseTex);
		auto rough = GetAssetManager()->GetTexture2DResource(roughnessTex);
		auto metal = GetAssetManager()->GetTexture2DResource(metallicTex);
		auto norm = GetAssetManager()->GetTexture2DResource(normalTex);
		if (diff) { diff->Bind(3); }
		else { Renderer::whiteTexture.Bind(3); }
		if (rough) { rough->Bind(4); }
		else { Renderer::whiteTexture.Bind(4); }
		if (metal) { metal->Bind(5); }
		else { Renderer::whiteTexture.Bind(5); }
		if (norm) { norm->Bind(6); }
		else { Renderer::whiteTexture.Bind(6); }
	}
	void Material::Update()
	{
		MaterialStruct Mstruct;
		Mstruct.albedo = diffuseColor;
		Mstruct.metallic = metallic;
		Mstruct.roughness = roughness;
		Mstruct.ID = -1;
		data.Update(&Mstruct, sizeof(MaterialStruct));
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
		YAML::Node diffuseColor = data["DiffuseColor"];
		YAML::Node metallic = data["Metallic"];
		YAML::Node roughness = data["Roughness"];
		YAML::Node diffuseTex = data["DiffuseTex"];
		YAML::Node metallicTex = data["MetallicTex"];
		YAML::Node roughnessTex = data["RoughnessTex"];
		YAML::Node normalTex = data["NormalTex"];
		auto assetMan = GetAssetManager();
		mat->diffuseColor = diffuseColor.as<DirectX::XMFLOAT4>();
		mat->diffuseTexName = diffuseTex.as<std::string>();
		mat->metallicTexName = metallicTex.as<std::string>();
		mat->roughnessTexName = roughnessTex.as<std::string>();
		mat->normalTexName = normalTex.as<std::string>();
		if(diffuseTex.as<std::string>() != "None")mat->diffuseTex = assetMan->CreateTexture2DAsset(std::string("assets/") + diffuseTex.as<std::string>());
		if(metallicTex.as<std::string>() != "None")mat->metallicTex = assetMan->CreateTexture2DAsset(std::string("assets/") + metallicTex.as<std::string>());
		if(roughnessTex.as<std::string>() != "None")mat->roughnessTex = assetMan->CreateTexture2DAsset(std::string("assets/") + roughnessTex.as<std::string>());
		if(normalTex.as<std::string>() != "None")mat->normalTex = assetMan->CreateTexture2DAsset(std::string("assets/") + normalTex.as<std::string>());
		mat->metallic = metallic.as<float>();
		mat->roughness = roughness.as<float>();
		mat->Initialize();
		mat->Update();
		return mat;
	}
}
