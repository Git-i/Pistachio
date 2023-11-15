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
#include "Pistachio\Asset\AssetManager.h"
#include "Pistachio/Renderer/EditorCamera.h"
namespace Pistachio {
	struct Light {
		DirectX::XMFLOAT4 positionxtype; // for directional lights this is direction and type
		DirectX::XMFLOAT4 colorxintensity;
		DirectX::XMFLOAT4 exData;
		DirectX::XMFLOAT4 rotation;
	};
	struct PassConstants
	{
		DirectX::XMFLOAT4X4 View;
		DirectX::XMFLOAT4X4 InvView;
		DirectX::XMFLOAT4X4 Proj;
		DirectX::XMFLOAT4X4 InvProj;
		DirectX::XMFLOAT4X4 ViewProj;
		DirectX::XMFLOAT4X4 InvViewProj;
		DirectX::XMFLOAT4 EyePosW;
		//float ShadowMapSize;
		DirectX::XMFLOAT2 RenderTargetSize;
		DirectX::XMFLOAT2 InvRenderTargetSize;
		float NearZ;
		float FarZ;
		float TotalTime;
		float DeltaTime;
		DirectX::XMMATRIX lightSpaceMatrix[16];
		DirectX::XMFLOAT4 numlights;
	};
	struct TransformData
	{
		DirectX::XMMATRIX transform;
		DirectX::XMMATRIX normal;
	};
	class Renderer {
	public:
		static void Shutdown();
		static void BeginScene(PerspectiveCamera* cam);
		static void BeginScene(RuntimeCamera* cam, const DirectX::XMMATRIX& transform);
		static void BeginScene(EditorCamera& cam);
		static void Init(const char* skybox);
		static void EndScene();
		static void Submit(Mesh* mesh, Shader* shader,  Material* mat, int ID);
		static void AddLight(const Light& light);
		inline static ShaderLibrary& GetShaderLibrary() { return shaderlib; }
		inline static ID3D11ShaderResourceView* GetFrambufferSRV() { return (ID3D11ShaderResourceView*)fbo.GetID().ptr; };
		inline static ID3D11ShaderResourceView* GetIrradianceFrambufferSRV() { return (ID3D11ShaderResourceView*)ifbo.GetID().ptr; };
		inline static ID3D11ShaderResourceView* GetPrefilterFrambufferSRV(int level) { return (ID3D11ShaderResourceView*)prefilter.GetID().ptr; };
		inline static ID3D11ShaderResourceView* GetBrdfSRV() { return (ID3D11ShaderResourceView*)BrdfTex.GetSRV().ptr; };
		static Material DefaultMaterial;
		static void OnEvent(Event& e) {
			if (e.GetEventType() == EventType::WindowResize)
				OnWindowResize((WindowResizeEvent&)e);
		}
		static void OnWindowResize(WindowResizeEvent& e)
		{
		}
		struct LD {
			Light lights[128];
		};
	private:
		static void CreateConstantBuffers();
		static void UpdatePassConstants();
	private:
		static RenderCubeMap fbo;
		static RenderCubeMap ifbo;
		static RenderCubeMap prefilter;
		static RenderTexture BrdfTex;
		static DirectX::XMMATRIX viewproj;
		static DirectX::XMVECTOR m_campos;
		static ShaderLibrary shaderlib;
		static ConstantBuffer MaterialCB;
		static ConstantBuffer PassCB;
		static ConstantBuffer LightCB;
		static std::vector<ConstantBuffer> TransformationBuffer;
		static struct CamerData { DirectX::XMMATRIX viewProjection; DirectX::XMMATRIX view;  DirectX::XMFLOAT4 viewPos; }CameraData;
		static Texture2D whiteTexture;
		static PassConstants passConstants;
		static LD LightData;
		static Light* lightIndexPtr;
		static Material* currentMat;
		static Shader* currentShader;
		friend class Scene;
		friend class Material;
	};
}
