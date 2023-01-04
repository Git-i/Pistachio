#include "ptpch.h"
#include "Camera.h"
#include "../Core/Application.h"
namespace Pistachio {
	OrthographicCamera::OrthographicCamera(float left, float right, float top, float bottom, float aspectratio)
		:m_projectionMatrix(DirectX::XMMatrixOrthographicLH(right-left * aspectratio, top-bottom * (1/aspectratio), 0.1f, 100.0f)), m_ViewMatrix(DirectX::XMMatrixIdentity())
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
	PerspectiveCamera::PerspectiveCamera(float fov, float nearPlane, float farPLane)
		:m_projectionMatrix(DirectX::XMMatrixPerspectiveFovLH(DirectX::XMConvertToRadians(fov), (float)(Application::Get().GetWindow().GetWidth()) / (float)(Application::Get().GetWindow().GetHeight()), nearPlane, farPLane)), m_ViewMatrix(DirectX::XMMatrixIdentity()), fov(DirectX::XMConvertToRadians(fov))
	{
		m_eye = { 0.0f, 0.0f, -5.0f };
		m_direction = { 0.0f, 0.0f, 0.0f };
		m_up.z = 0.0f;
		SetRotation(0.0f);
		m_ViewProjMatrix = m_ViewMatrix * m_projectionMatrix;
	}
	void PerspectiveCamera::RecalculateViewMatrix()
	{
		m_ViewMatrix = DirectX::XMMatrixLookAtLH(DirectX::XMLoadFloat3(&m_eye), DirectX::XMLoadFloat3(&m_direction), DirectX::XMLoadFloat3(&m_up));
		m_ViewProjMatrix = DirectX::XMMatrixTranspose(m_ViewMatrix * m_projectionMatrix);
	}
}