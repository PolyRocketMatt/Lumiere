#pragma once

#include "glm/glm.hpp"

#include <vector>

namespace Lumiere {

struct CameraMetadata {
	//	--- Geometry & Optics ---
	glm::vec3	position = glm::vec3(0.0f, 0.0f, 0.0f);
	glm::vec3	forwardDirection = glm::vec3(0.0f, 0.0f, -1.0f);
	glm::vec3	up = glm::vec3(0.0f, 1.0f, 0.0f);

	float		verticalFov = 45.0f;		
	float		focalLength;
	float		aperture;
	float		focusDistance;
	float		nearClip = 0.1f;
	float		farClip = 1.0f;

	//	--- Sensor ---
	float		sensorWidth;
	float		sensorHeight;
	float		sensorOpenTime;
	float		sensorCloseTime;

	//	--- Misc ---
	uint32_t	imageWidth = 0;
	uint32_t	imageHeight = 0;
	float		aspectRatio = 0;

	//  --- Real-Time ---
	bool		continuousRender = true;
};

struct OrbitCameraMetadata : public CameraMetadata {
	glm::vec3	target = glm::vec3(0.0f);
	float		distance = 5.0f;
	float		pitch = 0.0f;
	float		yaw = 0.0f;
	
	float		sensitivity = 1.0f;
	float		rotationSpeed = 0.1f;
	float		zoomSpeed = 1.25f;
	float		panSpeed = 0.1f;
};

struct FirstPersonCameraMetadata : public CameraMetadata {
	float		sensitivity = 0.002f;
	float		movementSpeed = 5.0f;
	float		rotationSpeed = 0.3f;
};

class BaseCamera {
public:
	BaseCamera(const CameraMetadata& metaData);
	~BaseCamera() = default;

	virtual bool OnUpdate(float timeStep);
	void OnResize(uint32_t width, uint32_t height);

	const glm::mat4& GetProjection() const { return m_Projection; }
	const glm::mat4& GetInverseProjection() const { return m_InverseProjection; }
	const glm::mat4& GetView() const { return m_View; }
	const glm::mat4& GetInverseView() const { return m_InverseView; }
	
	const glm::vec3& GetPosition() const { return m_Position; }
	const glm::vec3& GetForwardDirection() const { return m_ForwardDirection; }
	const glm::vec3& GetUp() const { return m_Up; }

	virtual void SetPosition(const glm::vec3 position, const glm::vec3 forwardDirection) = 0;

	const std::vector<glm::vec3>& GetRayDirections() const { return m_RayDirections; }
protected:
	void RecalculateProjection();
	void RecalculateView();
	void RecalculateRayDirections();
	
protected:
	glm::mat4				m_Projection{ 1.0f };
	glm::mat4				m_View{ 1.0f };
	glm::mat4				m_InverseProjection{ 1.0f };
	glm::mat4				m_InverseView{ 1.0f };

	glm::vec3				m_Position;
	glm::vec3				m_ForwardDirection;
	glm::vec3				m_RightDirection;
	glm::vec3				m_Up;

	float					m_VerticalFov;
	float					m_NearClip;
	float					m_FarClip;

	uint32_t				m_ImageWidth;
	uint32_t				m_ImageHeight;
	float					m_AspectRatio;

	bool					m_ContinuousRender;

	//	Cached ray directions
	std::vector<glm::vec3>	m_RayDirections;
};

class OrbitCamera : public BaseCamera {
public:
	OrbitCamera(const OrbitCameraMetadata& metaData);
	
	bool OnUpdate(float timeStep) override;

	void SetPosition(const glm::vec3 position, const glm::vec3 forwardDirection) override;

	void Rotate(const glm::vec2& delta, float timeStep);
	void Zoom(float delta, float timeStep);
	void Pan(const glm::vec2& delta, float timeStep);

private:
	glm::vec3				m_Target;
	float					m_Distance = 10.0f;
	float					m_Pitch = 0.0f;
	float					m_Yaw = 0.0f;

	float					m_Sensitivity;
	float					m_RotationSpeed;
	float					m_ZoomSpeed;
	float					m_PanSpeed;

	glm::vec2				m_LastMousePosition{ 0.0f, 0.0f };
};

class FirstPersonCamera : public BaseCamera {
public:
	FirstPersonCamera(const FirstPersonCameraMetadata& metaData);

	bool OnUpdate(float timeStep) override;

	void SetPosition(const glm::vec3 position, const glm::vec3 forwardDirection) override;
	
private:
	float					m_Sensitivity;
	float					m_MovementSpeed;
	float					m_RotationSpeed;

	glm::vec2				m_LastMousePosition{ 0.0f, 0.0f };
};

}