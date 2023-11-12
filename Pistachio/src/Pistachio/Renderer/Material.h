#pragma once
#include "Pistachio/Asset/Asset.h"
#include "Pistachio/Asset/RefCountedObject.h"
#include "Pistachio/Core/Math.h"
namespace Pistachio
{
	class Material : public RefCountedObject
	{
	public:
		Vector3 diffuseColor = Vector3(1.f, 1.f, 1.f);
		float metallic = 0.5f;
		float roughness = 0.5f;
		Asset diffuseTex;
		Asset normalTex;
		Asset metallicTex;
		Asset roughnessTex;
	};
}
