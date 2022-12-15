#pragma once
#include "RendererBase.h"
#include "Camera.h"
#include "Shader.h"
#include "Mesh.h"
namespace Pistachio {
	class Renderer {
	public:
		inline static void BeginScene(PerspectiveCamera* cam) {
			 m_viewProjectionMatrix = cam->GetViewProjectionMatrix();
		}
		static void EndScene();
		static void Submit(Buffer* buffer, Shader* shader, DirectX::XMMATRIX transform = DirectX::XMMatrixIdentity());
		static void Submit(Mesh* mesh, Shader* shader, DirectX::XMMATRIX transform = DirectX::XMMatrixIdentity());
	private:
		static DirectX::XMMATRIX m_viewProjectionMatrix;
	};
}
