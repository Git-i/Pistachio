#pragma once
#include "RendererBase.h"
#include "Camera.h"
#include "Shader.h"
namespace Pistachio {
	enum class CameraType
	{
		Perspective, Orthographic
	};
	struct  CamData {
		void* camera;
		CameraType cameratype;
	};
	class Renderer {
	public:
		inline static void BeginScene(PerspectiveCamera* cam) {
			 m_camera.camera = cam;
			 m_camera.cameratype = CameraType::Perspective;

		}
		inline static void BeginScene(OrthographicCamera* cam) {
			 m_camera.camera = cam;
			 m_camera.cameratype = CameraType::Orthographic;
		}
		static void EndScene();
		static void Submit(Buffer* buffer, Shader* shader, DirectX::XMMATRIX transform = DirectX::XMMatrixIdentity());
	private:
		static CamData m_camera;
	};
}
