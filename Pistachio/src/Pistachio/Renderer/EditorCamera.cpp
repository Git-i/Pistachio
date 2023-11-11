#include "ptpch.h"
#include "EditorCamera.h"

#include "Pistachio/Core/Input.h"
#include "Pistachio/Core/KeyCodes.h"


namespace Pistachio {

	EditorCamera::EditorCamera(float fov, float aspectRatio, float nearClip, float farClip)
		: m_FOV(fov), m_AspectRatio(aspectRatio), m_NearClip(nearClip), m_FarClip(farClip), RuntimeCamera(Matrix4::CreatePerspectiveFieldOfView(DirectX::XMConvertToRadians(fov), aspectRatio, nearClip, farClip))
	{
		UpdateView();
	}

	void EditorCamera::UpdateProjection()
	{
		m_AspectRatio = m_ViewportWidth / m_ViewportHeight;
		m_projection = Matrix4::CreatePerspectiveFieldOfView(Math::ToRadians(m_FOV), m_AspectRatio, m_NearClip, m_FarClip);
	}

	void EditorCamera::UpdateView()
	{
		// m_Yaw = m_Pitch = 0.0f; // Lock the camera's rotation
		m_Position = CalculatePosition();
		m_ViewMatrix = Matrix4::CreateLookAt(m_Position, m_FocalPoint, GetUpDirection());
		//Quaternion orientation = GetOrientation();
		//We Apply A Rigid-Body Transform Inverse Instead of a standard XMMatrixInverse 
		//m_ViewMatrix = DirectX::XMMatrixTranslationFromVector(DirectX::XMVectorNegate(m_Position)) * DirectX::XMMatrixTranspose(DirectX::XMMatrixRotationQuaternion(orientation));
	}

	std::pair<float, float> EditorCamera::PanSpeed() const
	{
		float x = std::min(m_ViewportWidth / 1000.0f, 2.4f); // max = 2.4f
		float xFactor = 0.0366f * (x * x) - 0.1778f * x + 0.3021f;

		float y = std::min(m_ViewportHeight / 1000.0f, 2.4f); // max = 2.4f
		float yFactor = 0.0366f * (y * y) - 0.1778f * y + 0.3021f;

		return { xFactor, yFactor };
	}

	float EditorCamera::RotationSpeed() const
	{
		return 0.8f;
	}

	float EditorCamera::ZoomSpeed() const
	{
		float distance = m_Distance * 0.2f;
		distance = std::max(distance, 0.0f);
		float speed = distance * distance;
		speed = std::min(speed, 10.0f); // max speed = 100
		return speed;
	}

	void EditorCamera::OnUpdate(float timestep)
	{
			Vector2 mouse = { (float)Input::GetMouseX(), (float)Input::GetMouseY() };
			if (FirstMouse) {
				m_InitialMousePosition = mouse;
				FirstMouse = false;
			}
			Vector2 delta = { (mouse.x - m_InitialMousePosition.x) * 0.003f , (mouse.y - m_InitialMousePosition.y) * 0.003f  };
			m_InitialMousePosition = mouse;

			if (Input::IsKeyPressed(PT_KEY_LCTRL) && Input::IsKeyPressed(PT_KEY_LBUTTON))
				MouseRotate(delta);
			else if (Input::IsKeyPressed(PT_KEY_LSHIFT) && Input::IsKeyPressed(PT_KEY_LBUTTON))
				MousePan(delta);
			else if (Input::IsKeyPressed(PT_KEY_LSHIFT) && Input::IsKeyPressed(PT_KEY_RBUTTON))
				MouseZoom(delta.y);

		UpdateView();
	}

	void EditorCamera::OnEvent(Event& e)
	{
		EventDispatcher dispatcher(e);
		dispatcher.Dispatch<MouseScrolledEvent>(BIND_EVENT_FN(EditorCamera::OnMouseScroll));
	}

	bool EditorCamera::OnMouseScroll(MouseScrolledEvent& e)
	{
		float delta = e.GetYOffset() * 0.01f;
		MouseZoom(delta);
		UpdateView();
		return false;
	}

	void EditorCamera::MousePan(const Vector2& delta)
	{
		auto [xSpeed, ySpeed] = PanSpeed();
		m_FocalPoint = m_FocalPoint - (GetRightDirection() * (delta.x * xSpeed * m_Distance));
		m_FocalPoint = (GetUpDirection() * (delta.y * ySpeed * m_Distance)) + m_FocalPoint;
	}

	void EditorCamera::MouseRotate(const Vector2& delta)
	{
		float yawSign = GetUpDirection().y < 0 ? -1.0f : 1.0f;
		m_Yaw += yawSign * delta.x * RotationSpeed();
		m_Pitch += delta.y * RotationSpeed();
	}

	void EditorCamera::MouseZoom(float delta)
	{
		float speed = ZoomSpeed();
		m_Distance -= delta * ZoomSpeed();
		if (m_Distance < 1.0f)
		{
			//m_FocalPoint = DirectX::XMVectorAdd(GetForwardDirection(), m_FocalPoint);
			m_Distance = 1.0f;
		}
	}

	Vector3 EditorCamera::GetUpDirection() const
	{
		return DirectX::XMVector3Rotate(Vector3::Up, GetOrientation());
	}

	Vector3 EditorCamera::GetRightDirection() const
	{
		return DirectX::XMVector3Rotate(Vector3::Forward, GetOrientation());
	}

	Vector3 EditorCamera::GetForwardDirection() const
	{
		return DirectX::XMVector3Rotate(Vector3(0.0f, 0.0f, -1.0f), GetOrientation());
	}

	Vector3 EditorCamera::CalculatePosition() const
	{
		return m_FocalPoint - (GetForwardDirection() * -m_Distance);
	}

	Quaternion EditorCamera::GetOrientation() const
	{
		return Quaternion::CreateFromYawPitchRoll(-m_Yaw, -m_Pitch, 0.0f);
	}

}