#pragma once
#include "Camera.h"
#include "Texture.h"
namespace Pistachio {
	class Renderer2D {
	public:
		static void Init();
		static void Shutdown();
		static void BeginScene(const OrthographicCamera& camera);
		static void EndScene();

		//Primitives
		static void DrawQuad(const DirectX::XMFLOAT3& position, const DirectX::XMFLOAT2& size, const DirectX::XMVECTOR& color);
		static void DrawQuad(const DirectX::XMFLOAT3& position, const DirectX::XMFLOAT2& size, const Texture& texture, const DirectX::XMVECTOR& color = DirectX::XMVectorSet(1.f, 1.f, 1.f, 1.f));
	};
}
