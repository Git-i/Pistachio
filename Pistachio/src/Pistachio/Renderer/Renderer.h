#pragma once
#include "RendererBase.h"
#include "Camera.h"
#include "Shader.h"
#include "Mesh.h"
#include "RenderTexture.h"
#include "Texture.h"
#include "Sampler.h"
namespace Pistachio {
	class Renderer {
	public:
		inline static void BeginScene(PerspectiveCamera* cam) {
			 m_camera = cam;
		}
		static void Init();
		static void EndScene();
		static void Submit(Buffer* buffer, Shader* shader, DirectX::XMMATRIX transform = DirectX::XMMatrixIdentity());
		static void Submit(Mesh* mesh, Shader* shader,  float* c, float m, float r, float ao, bool a, DirectX::XMMATRIX transform= DirectX::XMMatrixIdentity(), DirectX::XMMATRIX viewProjection = m_camera->GetViewProjectionMatrix());
		inline static ID3D11ShaderResourceView* GetFrambufferSRV() { return fbo.shaderResourceView; };
		inline static ID3D11ShaderResourceView* GetIrradianceFrambufferSRV() { return ifbo.shaderResourceView; };
		inline static ID3D11ShaderResourceView* GetPrefilterFrambufferSRV(int level) { return prefilter[level].shaderResourceView; };
		inline static ID3D11ShaderResourceView* GetBrdfSRV() { return pBrdfSRV; };
	private:
		static RenderCubeMap fbo;
		static RenderCubeMap ifbo;
		static RenderCubeMap prefilter[5];
		static ID3D11ShaderResourceView* pBrdfSRV;
		static ID3D11RenderTargetView* pBrdfRTV;
		static PerspectiveCamera* m_camera;
	};
}
