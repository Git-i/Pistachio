#pragma once
#include "RendererBase.h"
#include "Camera.h"
#include "Shader.h"
namespace Pistachio {
	class Renderer {
	public:
		static void BeginScene(OrthographicCamera& cam);
		static void EndScene();
		static void Submit(Buffer& buffer, Shader& shader);
	private:
		static DirectX::XMMATRIX ViewProjectionMatrix;
	};
}
