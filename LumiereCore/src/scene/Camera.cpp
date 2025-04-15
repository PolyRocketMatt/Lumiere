#include "Camera.h"
#include "input/Input.h"

#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/quaternion.hpp"

#define GLM_ENABLE_EXPERIMENTAL
#include "glm/gtx/quaternion.hpp"

#include <iostream>

namespace Lumiere {

//	
//	BaseCamera
//
BaseCamera::BaseCamera(const CameraMetadata& metaData)
	: m_Position(metaData.position),
	m_ForwardDirection(metaData.forwardDirection),
	m_RightDirection(glm::cross(metaData.forwardDirection, metaData.up)),
	m_Up(metaData.up),
	m_VerticalFov(metaData.verticalFov),
	m_NearClip(metaData.nearClip),
	m_FarClip(metaData.farClip),
	m_ImageWidth(metaData.imageWidth),
	m_ImageHeight(metaData.imageHeight),
	m_AspectRatio((float)metaData.imageWidth / metaData.imageHeight),
	m_ContinuousRender(metaData.continuousRender) { }

bool BaseCamera::OnUpdate(float timeStep) {
	return false;
}

void BaseCamera::OnResize(uint32_t width, uint32_t height) {
	if (width == m_ImageWidth && height == m_ImageHeight)
		return;

	m_ImageWidth = width;
	m_ImageHeight = height;
	m_AspectRatio = (float)width / height;

	RecalculateProjection();
	RecalculateRayDirections();
}

void BaseCamera::RecalculateProjection() {
	m_Projection = glm::perspectiveFov(glm::radians(m_VerticalFov), (float)m_ImageWidth, (float)m_ImageHeight, m_NearClip, m_FarClip);	
	m_InverseProjection = glm::inverse(m_Projection);
}

void BaseCamera::RecalculateView() {
	m_View = glm::lookAt(m_Position, m_Position + m_ForwardDirection, glm::vec3(0, 1, 0));
	m_InverseView = glm::inverse(m_View);
}

void BaseCamera::RecalculateRayDirections() {
	m_RayDirections.resize(m_ImageWidth * m_ImageHeight);

	for (uint32_t y = 0; y < m_ImageHeight; y++) {
		for (uint32_t x = 0; x < m_ImageWidth; x++) {
			glm::vec2 coord = { (float)x / m_ImageWidth, (float)y / m_ImageHeight };

			//	Map to UV [-1, 1]
			coord = coord * 2.0f - 1.0f;

			//	Calculate ray direction from origin in world space
			glm::vec4 target = m_InverseProjection * glm::vec4(coord.x, coord.y, 1, 1);
			glm::vec3 rayDirection = glm::vec3(m_InverseView * glm::vec4(glm::normalize(glm::vec3(target) / target.w), 0.0f));

			m_RayDirections[x + y * m_ImageWidth] = rayDirection;
		}
	}
}

//	
//	OrbitCamera
//
OrbitCamera::OrbitCamera(const OrbitCameraMetadata& metaData)
	: BaseCamera(metaData),
	m_Target(metaData.target),
	m_Distance(metaData.distance),
	m_Pitch(metaData.pitch),
	m_Yaw(metaData.yaw),
	m_Sensitivity(metaData.sensitivity),
	m_RotationSpeed(metaData.rotationSpeed),
	m_ZoomSpeed(metaData.zoomSpeed),
	m_PanSpeed(metaData.panSpeed) { }

bool OrbitCamera::OnUpdate(float timeStep) {
	glm::vec2 mousePos = Input::GetMousePosition();
	glm::vec2 movementDelta = (mousePos - m_LastMousePosition) * m_Sensitivity;
	m_LastMousePosition = mousePos;

	bool moved = false;
	
	//	Movement
	//	Pan
	if (Input::IsMouseButtonDown(MouseButton::Middle) && Input::IsKeyDown(KeyCode::LeftShift)) {
		Pan(movementDelta, timeStep);
		moved = true;
	}
	
	//	Orbit
	else if (Input::IsMouseButtonDown(MouseButton::Middle)) {
		Rotate(movementDelta, timeStep);
		moved = true;
	}

	//	Zoom
	else if (Input::HasScrollDelta()) {
		float scrollDelta = Input::GetMouseScrollDelta().y;
		Zoom(scrollDelta, timeStep);
		moved = true;
	}

	//	Clamp pitch to prevent flipping
	constexpr float pitchLimit = glm::radians(89.0f);
	m_Pitch = glm::clamp(m_Pitch, -pitchLimit, pitchLimit);

	//	Spherical coordinate transformation
	float x = m_Distance * glm::cos(m_Pitch) * glm::sin(m_Yaw);
	float y = m_Distance * glm::sin(m_Pitch);
	float z = m_Distance * glm::cos(m_Pitch) * glm::cos(m_Yaw);

	m_Position = m_Target + glm::vec3(x, y, z);
	m_ForwardDirection = glm::normalize(m_Target - m_Position);
	m_RightDirection = glm::cross(m_ForwardDirection, m_Up);

	if (moved) {
		RecalculateView();
		RecalculateRayDirections();
	}

	return moved;
}

void OrbitCamera::SetPosition(const glm::vec3 position, const glm::vec3 forwardDirection) {
	m_Target = glm::vec3(0.0f);
	m_Position = position;
	m_ForwardDirection = glm::normalize(m_Target - m_Position);
	m_Pitch = 0.0f;
	m_Yaw = 0.0f;

	RecalculateView();
	RecalculateRayDirections();
}

void OrbitCamera::Rotate(const glm::vec2& delta, float timeStep) {
	m_Yaw += -delta.x * m_RotationSpeed * timeStep;
	m_Pitch += delta.y * m_RotationSpeed * timeStep;
}

void OrbitCamera::Zoom(float delta, float timeStep) {
	m_Distance *= (1.0f + -delta * m_ZoomSpeed * timeStep);
	m_Distance = glm::max(m_Distance, 0.1f);
}

void OrbitCamera::Pan(const glm::vec2& delta, float timeStep) {
	glm::vec3 rightDirection = glm::normalize(glm::cross(m_Target - m_Position, m_Up));
	
	m_Target += -rightDirection * delta.x * m_PanSpeed * m_Distance * timeStep;
	m_Target += m_Up * delta.y * m_PanSpeed * m_Distance * timeStep;
}

//
//	FirstPersonCamera
//
FirstPersonCamera::FirstPersonCamera(const FirstPersonCameraMetadata& metaData)
	: BaseCamera(metaData),
	m_Sensitivity(metaData.sensitivity),
	m_MovementSpeed(metaData.movementSpeed),
	m_RotationSpeed(metaData.rotationSpeed) { }

bool FirstPersonCamera::OnUpdate(float timeStep) {
	glm::vec2 mousePos = Input::GetMousePosition();
	glm::vec2 movementDelta = (mousePos - m_LastMousePosition) * m_Sensitivity;
	m_LastMousePosition = mousePos;

	if (!Input::IsMouseButtonDown(MouseButton::Right)) {
		Input::SetCursorMode(CursorMode::Normal);
		return false;
	}

	Input::SetCursorMode(CursorMode::Locked);
	bool moved = false;

	//	Movement
	//	Forward
	if (Input::IsKeyDown(KeyCode::W)) {
		m_Position += m_ForwardDirection * m_MovementSpeed * timeStep;
		moved = true;
	}

	//	Backward
	else if (Input::IsKeyDown(KeyCode::S)) {
		m_Position -= m_ForwardDirection * m_MovementSpeed * timeStep;
		moved = true;
	}

	//	Right
	else if (Input::IsKeyDown(KeyCode::D)) {
		m_Position += m_RightDirection * m_MovementSpeed * timeStep;
		moved = true;
	}

	//	Left
	else if (Input::IsKeyDown(KeyCode::A)) {
		m_Position -= m_RightDirection * m_MovementSpeed * timeStep;
		moved = true;
	}

	//	Up
	else if (Input::IsKeyDown(KeyCode::E)) {
		m_Position += m_Up * m_MovementSpeed * timeStep;
		moved = true;
	}

	else if (Input::IsKeyDown(KeyCode::Q)) {
		m_Position -= m_Up * m_MovementSpeed * timeStep;
		moved = true;
	}

	//	Rotation
	if (movementDelta.x != 0.0f || movementDelta.y != 0.0f) {
		float pitchDelta = movementDelta.y * m_RotationSpeed;
		float yawDelta = movementDelta.x * m_RotationSpeed;
		
		glm::quat q = glm::normalize(glm::cross(glm::angleAxis(-pitchDelta, m_RightDirection), glm::angleAxis(-yawDelta, m_Up)));
		m_ForwardDirection = glm::rotate(q, m_ForwardDirection);
		m_RightDirection = glm::cross(m_ForwardDirection, m_Up);

		moved = true;
	}
	
	if (moved) {
		RecalculateView();
		RecalculateRayDirections();
	}

	return moved;
}

void FirstPersonCamera::SetPosition(const glm::vec3 position, const glm::vec3 forwardDirection) {
	m_Position = position;
	m_ForwardDirection = forwardDirection;

	RecalculateView();
	RecalculateRayDirections();
}

}