#pragma once

namespace Pistachio {
	class OrthographicCamera {
	public:
		OrthographicCamera(float left, float right, float top, float bottom, float aspectratio);
		inline DirectX::XMFLOAT3 GetPosition() { return m_Position; }
		inline float GetRotation() { return m_Rotation; }
		inline void SetPosition(float x, float y, float z) { m_Position.x = x; m_Position.y = y; m_Position.z = z; RecalculateViewMatrix(); }
		inline void SetPosition(DirectX::XMFLOAT3 pos) { m_Position = pos; RecalculateViewMatrix(); }
		inline void SetRotation(float Rotation) { m_Rotation = DirectX::XMConvertToRadians(Rotation); RecalculateViewMatrix(); }
		inline DirectX::XMMATRIX GetProjectionMatrix() const { return m_projectionMatrix; }
		inline DirectX::XMMATRIX GetViewMatrix() const { return m_ViewMatrix; }
		inline DirectX::XMMATRIX GetViewProjectionMatrix() const { return m_ViewProjMatrix; }
		void Zoom(float amount, float aspect);
	private:
		void RecalculateViewMatrix();
	private:
		float width, height;
		DirectX::XMMATRIX m_projectionMatrix;
		DirectX::XMMATRIX m_ViewMatrix;
		DirectX::XMMATRIX m_ViewProjMatrix;
		DirectX::XMFLOAT3 m_Position;
		float m_Rotation = 0.0f;
	};
	class PerspectiveCamera {
	public:
		PerspectiveCamera(float fov, float nearPlane, float farPLane);
		inline DirectX::XMFLOAT3 GetPosition() { return m_eye; }
		inline float GetRotation() { return m_Rotation; }
		inline void SetRotation(float Rotation) { m_Rotation = Rotation; m_up.x = DirectX::XMScalarSin(Rotation); m_up.y = DirectX::XMScalarCos(Rotation); RecalculateViewMatrix(); }
		inline void SetPosition(float x, float y, float z) { m_eye.x = x; m_eye.y = y; m_eye.z = z; RecalculateViewMatrix(); }
		inline void SetPosition(DirectX::XMFLOAT3 pos) { m_eye = pos; RecalculateViewMatrix(); }
		inline void SetLookAt(DirectX::XMFLOAT3 pos) { m_direction = pos; RecalculateViewMatrix(); }
		inline void SetLookAt(float x, float y, float z) { m_direction.x = x; m_direction.y = y; m_direction.z = z; RecalculateViewMatrix(); }
		inline void SetViewMat(DirectX::XMMATRIX view) { m_ViewMatrix = view; m_ViewProjMatrix = DirectX::XMMatrixTranspose(m_ViewMatrix * m_projectionMatrix); }
		inline DirectX::XMMATRIX GetProjectionMatrix() { return m_projectionMatrix; }
		inline DirectX::XMMATRIX GetViewMatrix() { return m_ViewMatrix; }
		inline DirectX::XMMATRIX GetViewProjectionMatrix() { return m_ViewProjMatrix; }
		inline void ChangeAspectRatio(float aspectratio) { m_projectionMatrix = DirectX::XMMatrixPerspectiveFovLH(fov, aspectratio, 0.1f, 100.0f); m_ViewProjMatrix = DirectX::XMMatrixMultiplyTranspose(m_ViewMatrix, m_projectionMatrix); }
	private:
		void RecalculateViewMatrix();
	private:
		DirectX::XMMATRIX m_projectionMatrix;
		DirectX::XMMATRIX m_ViewMatrix;
		DirectX::XMMATRIX m_ViewProjMatrix;
		DirectX::XMFLOAT3 m_eye;
		DirectX::XMFLOAT3 m_direction;
		DirectX::XMFLOAT3 m_up;
		float fov;
		float m_Rotation;
	};

	class RuntimeCamera {
	public:
		RuntimeCamera() : m_projection(DirectX::XMMatrixIdentity()){}
		RuntimeCamera(const DirectX::XMMATRIX& projection) : m_projection(projection) {}
		const DirectX::XMMATRIX& GetProjection() const { return m_projection; }
		const DirectX::XMMATRIX& GetProjection() { return m_projection; }
	protected:
		DirectX::XMMATRIX m_projection;
	};
	class SceneCamera : public RuntimeCamera {
	public:
		enum class ProjectionType {Perspective = 0, Orthographic};
	public:
		SceneCamera();
		void SetPerspective(float fov, float nearplane, float farplane);
		void SetViewportSize(unsigned int width, unsigned int height);
		void SetOrthographic(float size, float nearplane, float farplane);
		void SetProjectionType(ProjectionType type);
		inline ProjectionType GetProjectionType() const { return m_type; }
		inline float GetOrthoNear() {return m_orthonear ;}
		inline float GetOrthoFar()  {return m_orthofar  ;}
		inline float GetOrthoSize() {return m_orthosize ;}
		inline float GetPerspNear() {return m_perspnear ;}
		inline float GetPerspFar()  {return m_perspfar  ;}
		inline float GetPerspSize() {return m_perspsize ;}
	private:
		float m_orthonear = 0.1f;
		float m_orthofar = 100.0f;
		float m_orthosize = 10.f;
		float m_perspnear = 0.1f;
		float m_perspfar  = 100.f;
		float m_perspsize = DirectX::XMConvertToRadians(75.f);
		float m_aspectratio = 1.f;
		ProjectionType m_type;
		friend class SceneHierarchyPanel;
	};
}
