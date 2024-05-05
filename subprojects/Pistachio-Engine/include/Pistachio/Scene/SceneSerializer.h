#pragma once
#include "Scene.h"
namespace Pistachio {
	class SceneSerializer {
	public:
		SceneSerializer(const Ref<Scene>& scene);
		void Serialize(const std::string& filepath);
		void DeSerialize(const std::string& filepath);
	private:
		Ref<Scene> m_Scene;
	};
}
