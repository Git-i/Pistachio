#pragma once
#include "Pistachio\Renderer\Material.h"
#include "Pistachio\Renderer\RenderTexture.h"
#include "Pistachio\Renderer\Camera.h"
namespace Pistachio
{
	class MaterialEditorPanel
	{
	public:
		MaterialEditorPanel();
		void OnImGuiRender();
		bool activated = true;
		RenderTexture previewRT;
		Asset mat;
		Asset sphere;
		RuntimeCamera cam;
		ConstantBuffer cb;
		std::string filepath;
	};
}
