#pragma once
#include "Camera.h"
#include "Texture.h"
#include "Pistachio/Scene/Components.h"
#include "Pistachio/Renderer/EditorCamera.h"
namespace Pistachio {
	class PISTACHIO_API Renderer2D {
	public:
		static void Init();
		static void BeginScene(const OrthographicCamera& camera);
		static void BeginScene(const RuntimeCamera& camera, const DirectX::XMMATRIX& transform);
		static void BeginScene(const EditorCamera& camera);
		static void EndScene();
		static void Flush();
		//Primitives
		static void DrawQuad(const DirectX::XMFLOAT3& position, const DirectX::XMFLOAT2& size, const DirectX::XMFLOAT4& color);
		static void DrawSprite(const DirectX::XMMATRIX& transform, const SpriteRendererComponent& src, int ID);
		static void DrawQuad(const DirectX::XMFLOAT3& position, const DirectX::XMFLOAT2& size, const Ref<Texture2D>& texture, const DirectX::XMFLOAT4& color = { 1.f, 1.f, 1.f, 1.f });
		static void DrawQuad(const DirectX::XMMATRIX& transform, const DirectX::XMFLOAT4& color);
		static void DrawQuad(const DirectX::XMMATRIX& transform, const Ref<Texture2D>& texture, const DirectX::XMFLOAT4& color = { 1.f, 1.f, 1.f, 1.f });
		static void DrawRotatedQuad(const DirectX::XMFLOAT3& position, const DirectX::XMFLOAT2& size, const DirectX::XMFLOAT4& color, float angle);
		static void DrawRotatedQuad(const DirectX::XMFLOAT3& position, const DirectX::XMFLOAT2& size, const Ref<Texture2D>& texture, const DirectX::XMFLOAT4& color = { 1.f, 1.f, 1.f, 1.f });
	private:
		static void FlushAndReset();
	};

}
