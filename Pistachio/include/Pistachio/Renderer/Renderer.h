#pragma once
#include "RendererBase.h"
#include "Camera.h"
#include "Shader.h"
#include "Model.h"
#include "RenderTexture.h"
#include "Texture.h"
#include "Sampler.h"
#include "Pistachio/Event/Event.h"
#include "Pistachio/Event/ApplicationEvent.h"

#include "Pistachio/Renderer/EditorCamera.h"
namespace Pistachio {
	struct Light {
		DirectX::XMFLOAT4 positionxtype; // for directional lights this is direction and type
		DirectX::XMFLOAT4 colorxintensity;
		DirectX::XMFLOAT4 exData;
		DirectX::XMFLOAT4 rotation;
	};
	class Renderer {
	public:
		static void Shutdown();
		static void BeginScene(PerspectiveCamera* cam);
		static void BeginScene(RuntimeCamera* cam, const DirectX::XMMATRIX& transform);
		static void BeginScene(EditorCamera& cam);
		static void Init(const char* skybox);
		static void EndScene();
		static void Submit(Mesh* mesh, Shader* shader,  float* c, float m, float r, int ID, const DirectX::XMMATRIX& transform = DirectX::XMMatrixIdentity(), const DirectX::XMMATRIX& viewProjection = viewproj);
		static void AddLight(const Light& light);
		inline static ShaderLibrary& GetShaderLibrary() { return shaderlib; }
		inline static ID3D11ShaderResourceView* GetFrambufferSRV() { return fbo.GetSRV(); };
		inline static ID3D11ShaderResourceView* GetIrradianceFrambufferSRV() { return ifbo.GetSRV(); };
		inline static ID3D11ShaderResourceView* GetPrefilterFrambufferSRV(int level) { return prefilter.GetSRV(); };
		inline static ID3D11ShaderResourceView* GetBrdfSRV() { return pBrdfSRV; };
		static void OnEvent(Event& e) {
			if (e.GetEventType() == EventType::WindowResize)
				OnWindowResize((WindowResizeEvent&)e);
		}
		static void OnWindowResize(WindowResizeEvent& e)
		{
		}
		struct ShadowData {
			DirectX::XMMATRIX lightSpaceMatrix[16];
			DirectX::XMFLOAT4 numlights;
		};
		struct LD {
			Light lights[128];
		};
	private:
		static RenderCubeMap fbo;
		static RenderCubeMap ifbo;
		static RenderCubeMap prefilter;
		static ID3D11ShaderResourceView* pBrdfSRV;
		static ID3D11RenderTargetView* pBrdfRTV;
		static DirectX::XMMATRIX viewproj;
		static DirectX::XMVECTOR m_campos;
		static ShaderLibrary shaderlib;
		static ConstantBuffer MaterialCB;
		static ConstantBuffer CameraCB;
		static ConstantBuffer LightCB;
		static ConstantBuffer ShadowCB;
		static ConstantBuffer TransformationBuffer;
		static struct CamerData { DirectX::XMMATRIX viewProjection; DirectX::XMMATRIX view;  DirectX::XMFLOAT4 viewPos; }CameraData;
		static Texture2D whiteTexture;
		static ShadowData shadowData;
		static LD LightData;
		static Light* lightIndexPtr;
		friend class Scene;
	};
}
