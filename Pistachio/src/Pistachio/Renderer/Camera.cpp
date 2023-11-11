#include "ptpch.h"
#include "Camera.h"
#include "../Core/Application.h"
namespace Pistachio {
	OrthographicCamera::OrthographicCamera(float left, float right, float top, float bottom, float aspectratio)
		:m_projectionMatrix(DirectX::XMMatrixOrthographicLH(right-left * aspectratio, top-bottom, 0.1f, 100.0f)), m_ViewMatrix(DirectX::XMMatrixIdentity())
	{
		width = right - left;
		height = top - bottom;
		m_ViewProjMatrix = DirectX::XMMatrixTranspose(m_ViewMatrix * m_projectionMatrix);
		m_Position.x = 0;
		m_Position.y = 0;
		m_Position.z = 0;

	}
	void OrthographicCamera::Zoom(float amount, float aspect)
	{
		width += amount;
		height += amount;
		if(!DirectX::XMScalarNearEqual(width, 0.f, 0.00001f))
			m_projectionMatrix = DirectX::XMMatrixOrthographicLH((width) * aspect, (height), 0.1f, 100.f);
		RecalculateViewMatrix();
	}
	void OrthographicCamera::RecalculateViewMatrix()
	{
		m_ViewMatrix = DirectX::XMMatrixInverse(nullptr, (DirectX::XMMatrixRotationZ(m_Rotation) * DirectX::XMMatrixTranslationFromVector(DirectX::XMLoadFloat3(reinterpret_cast<DirectX::XMFLOAT3*>(&m_Position)))));
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
	SceneCamera::SceneCamera()
	{
		m_type = ProjectionType::Perspective;
		SetPerspective(m_perspsize, m_perspnear, m_perspfar);
	}
	void SceneCamera::SetPerspective(float fov, float nearplane, float farplane)
	{
		if (DirectX::XMScalarNearEqual(nearplane, farplane, 0.1f))
			farplane += 0.5f;
		m_type = ProjectionType::Perspective;
		m_perspsize = fov;
		m_perspnear = nearplane;
		m_perspfar = farplane;
		m_projection = Matrix4::CreatePerspectiveFieldOfView(m_perspsize, m_aspectratio, m_perspnear, m_perspfar);
	}
	void SceneCamera::SetViewportSize(unsigned int width, unsigned int height)
	{
		m_aspectratio = (float)width / (float)height;
		if (m_type == ProjectionType::Orthographic) {
			float width = m_orthosize * m_aspectratio;
			float height = m_orthosize;
			m_projection = Matrix4::CreateOrthographic(width, height, m_orthonear, m_orthofar); 
		}
		else
		{
			m_projection = Matrix4::CreatePerspectiveFieldOfView(m_perspsize, m_aspectratio, m_perspnear, m_perspfar);
		}
	}
	void SceneCamera::SetOrthographic(float size, float nearplane, float farplane)
	{
		m_type = ProjectionType::Orthographic;
		m_orthosize = size;
		m_orthonear = nearplane;
		m_orthofar = farplane;
		float width = m_orthosize * m_aspectratio;
		float height = m_orthosize;
		m_projection = Matrix4::CreateOrthographic(width, height, m_orthonear, m_orthofar);
	}
	void SceneCamera::SetProjectionType(ProjectionType type)
	{
		if (type == ProjectionType::Perspective)
			SetPerspective(m_perspsize, m_perspnear, m_perspfar);
		else
			SetOrthographic(m_orthosize, m_orthonear, m_orthofar);
	}
}