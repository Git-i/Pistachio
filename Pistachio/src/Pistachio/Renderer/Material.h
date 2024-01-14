#pragma once
#include <string>
#include "Pistachio/Asset/Asset.h"
#include "Pistachio/Asset/RefCountedObject.h"
#include "Pistachio/Core/Math.h"
#include "../Renderer/Shader.h"
namespace Pistachio
{
	class PISTACHIO_API Material : public RefCountedObject
	{
	public:
		Vector4 diffuseColor = Vector4(1.f, 1.f, 1.f, 1.f);
		float metallic = 0.5f;
		float roughness = 0.5f;
		Asset diffuseTex;
		Asset normalTex;
		Asset metallicTex;
		Asset roughnessTex;
		bool bDirty = true;
		//editor specific
		std::string diffuseTexName = "None";
		std::string normalTexName = "None";
		std::string metallicTexName = "None";
		std::string roughnessTexName = "None";
		void Initialize();
	public:
		void Bind();
		void Update();
	private:
		ConstantBuffer data;
	public:
		static Material* Create(const char* filepath);
	};

	class PISTACHIO_API MaterialSerializer
	{
	public:
		void Serialize(const std::string& filepath, const Material& mat);
	};
}
