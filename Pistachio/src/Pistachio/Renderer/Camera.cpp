#include "ptpch.h"
#include "Camera.h"

namespace Pistachio {
	OrthographicCamera::OrthographicCamera(float left, float right, float top, float bottom)
		:m_projectionMatrix(DirectX::XMMatrixOrthographicLH(right-left, top-bottom, -1.0, 1.0)), m_ViewMatrix(DirectX::XMMatrixIdentity())
	{
		m_ViewProjMatrix = m_ViewMatrix * m_projectionMatrix;
		m_Position.x = 0;
		m_Position.y = 0;
		m_Position.z = 0;

	}
	void OrthographicCamera::RecalculateViewMatrix()
	{
		m_ViewMatrix = DirectX::XMMatrixInverse(nullptr, (DirectX::XMMatrixRotationZ(m_Rotation) * DirectX::XMMatrixTranslationFromVector(DirectX::XMLoadFloat3(&m_Position))));
		m_ViewProjMatrix = DirectX::XMMatrixTranspose(m_ViewMatrix * m_projectionMatrix);
	}
}