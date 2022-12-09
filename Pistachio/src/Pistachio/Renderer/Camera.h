#pragma once

namespace Pistachio {
	class OrthographicCamera {
	public:
		OrthographicCamera(float left, float right, float top, float bottom);
		inline DirectX::XMFLOAT3 GetPosition() { return m_Position; }
		inline float GetRotation() { return m_Rotation; }
		inline void SetPosition(float x, float y, float z) { m_Position.x = x; m_Position.y = y; m_Position.z = z; RecalculateViewMatrix(); }
		inline void SetPosition(DirectX::XMFLOAT3 pos) { m_Position = pos; RecalculateViewMatrix(); }
		inline void SetRotation(float Rotation) { m_Rotation = DirectX::XMConvertToRadians(Rotation); RecalculateViewMatrix(); }
		inline DirectX::XMMATRIX GetProjectionMatrix() { return m_projectionMatrix; }
		inline DirectX::XMMATRIX GetViewMatrix() { return m_ViewMatrix; }
		inline DirectX::XMMATRIX GetViewProjectionMatrix() { return m_ViewProjMatrix; }
	private:
		void RecalculateViewMatrix();
	private:
		DirectX::XMMATRIX m_projectionMatrix;
		DirectX::XMMATRIX m_ViewMatrix;
		DirectX::XMMATRIX m_ViewProjMatrix;
		DirectX::XMFLOAT3 m_Position;
		float m_Rotation = 0.0f;
	};
}