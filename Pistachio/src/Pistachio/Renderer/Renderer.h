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

namespace Pistachio {
	
	class Renderer {
	public:
		static void Shutdown();
		inline static void BeginScene(PerspectiveCamera* cam, RenderTexture* target = 0, int numtextures = 1) {
			 m_camera = cam;
			 for (char i = 0; i < numtextures; ++i) {
				 m_target[i] = target[i].GetRTV();
				 float clear[4] = { 1.f, 0.f, 0.f, 0.f };
				 target[i].Clear(clear);
			 }
			 m_NumRenderTextures = numtextures;
			 m_pDSV = target[0].GetDSV();
		}
		static void Init(const char* skybox);
		static void EndScene();
		static void Submit(Buffer* buffer, Shader* shader, DirectX::XMMATRIX transform = DirectX::XMMatrixIdentity());
		static void Submit(Mesh* mesh, Shader* shader,  float* c, float m, float r, float ao, const DirectX::XMMATRIX& transform = DirectX::XMMatrixIdentity(), const DirectX::XMMATRIX& viewProjection = m_camera->GetViewProjectionMatrix());
		
		inline static ID3D11ShaderResourceView* GetFrambufferSRV() { return fbo.GetSRV(); };
		inline static ID3D11ShaderResourceView* GetIrradianceFrambufferSRV() { return ifbo.GetSRV(); };
		inline static ID3D11ShaderResourceView* GetPrefilterFrambufferSRV(int level) { return prefilter.GetSRV(); };
		inline static ID3D11ShaderResourceView* GetBrdfSRV() { return pBrdfSRV; };
		static void OnEvent(Event& e) {
			if (m_target)
			if (e.GetEventType() == EventType::WindowResize)
				OnWindowResize((WindowResizeEvent&)e);
		}
		static void OnWindowResize(WindowResizeEvent& e)
		{
			int a = 5;
		}
	private:
		static RenderCubeMap fbo;
		static RenderCubeMap ifbo;
		static RenderCubeMap prefilter;
		static ID3D11ShaderResourceView* pBrdfSRV;
		static ID3D11RenderTargetView* pBrdfRTV;
		static PerspectiveCamera* m_camera;
		static ID3D11RenderTargetView* m_target[8];
		static ID3D11DepthStencilView* m_pDSV;
		static int m_NumRenderTextures;
	};
}
