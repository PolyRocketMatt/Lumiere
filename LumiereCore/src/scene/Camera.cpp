#include "Camera.h"
#include "input/Input.h"

#include "glm/gtc/matrix_transform.hpp";

namespace Lumiere {

//	
//	BaseCamera
//
BaseCamera::BaseCamera(const CameraMetadata& metaData)
	: m_Position(metaData.s_Position),
	m_Direction(metaData.s_Direction),
	m_Up(metaData.s_Up),
	m_VerticalFov(metaData.s_VerticalFov),
	m_NearClip(metaData.s_NearClip),
	m_FarClip(metaData.s_FarClip),
	m_ImageWidth(metaData.s_ImageWidth),
	m_ImageHeight(metaData.s_ImageHeight),
	m_AspectRatio((float)m_ImageWidth / m_ImageHeight),
	m_ContinuousRender(metaData.s_ContinuousRender) {
}

void BaseCamera::OnResize(uint32_t width, uint32_t height) {
	if (width == m_ImageWidth && height == m_ImageHeight)
		return;

	m_ImageWidth = width;
	m_ImageHeight = height;

	RecalculateProjection();
	RecalculateRayDirections();
}

void BaseCamera::RecalculateProjection() {
	m_Projection = glm::perspectiveFov(glm::radians(m_VerticalFov), (float)m_ImageWidth, (float)m_ImageHeight, m_NearClip, m_FarClip);
	m_InverseProjection = glm::inverse(m_Projection);
}

void BaseCamera::RecalculateView() {
	m_View = glm::lookAt(m_Position, m_Position + m_Direction, glm::vec3(0, 1, 0));
	m_InverseView = glm::inverse(m_View);
}

void BaseCamera::RecalculateRayDirections() {
	m_RayDirections.resize(m_ImageWidth * m_ImageHeight);

	for (uint32_t y = 0; y < m_ImageHeight; y++) {
		for (uint32_t x = 0; x < m_ImageWidth; x++) {
			glm::vec2 coord = { (float)x / m_ImageWidth, (float)y / m_ImageHeight };

			//	Map to UV [-1, 1]
			coord = coord * 2.0f - 1.0f;

			//	Aspect ratio
			coord *= m_AspectRatio;

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
	m_Target(metaData.s_Target),
	m_Distance(metaData.s_Distance),
	m_Pitch(metaData.s_Pitch),
	m_Yaw(metaData.s_Yaw),
	m_RotationSpeed(metaData.s_RotationSpeed),
	m_ZoomSpeed(metaData.s_ZoomSpeed),
	m_PanSpeed(metaData.s_PanSpeed) {
}

bool OrbitCamera::OnUpdate(float timeStep) {
	glm::vec2 mousePos = Input::GetMousePosition();
	glm::vec2 movementDelta = mousePos - m_LastMousePosition;
	m_LastMousePosition = mousePos;

	bool moved = false;
	
	//	Movement
	//	Pan
	if (Input::IsMouseButtonDown(MouseButton::Middle) && Input::IsKeyDown(KeyCode::LeftShift)) {
		Pan(movementDelta);
		moved = true;
	}
	
	//	Orbit
	else if (Input::IsMouseButtonDown(MouseButton::Middle)) {
		Rotate(movementDelta);
		moved = true;
	}

	else if (Input::HasScrollDelta()) {
		float scrollDelta = Input::GetMouseScrollDelta().y;
		Zoom(scrollDelta);
		moved = true;
	}

	constexpr float pitchLimit = glm::radians(89.0f);
	m_Pitch = glm::clamp(m_Pitch, -m_Pitch, pitchLimit);

	//	Convert spherical coordinates to cartesian
	m_Position = glm::vec3(
		m_Target.x + m_Distance * std::cos(m_Pitch) * std::sin(m_Yaw),
		m_Target.y + m_Distance * std::sin(m_Pitch),
		m_Target.z + m_Distance * std::cos(m_Pitch) * std::cos(m_Yaw)
	);

	//	Look at target
	m_View = glm::lookAt(m_Position, m_Target, m_Up);
}

void OrbitCamera::Rotate(const glm::vec2& delta) {
	m_Yaw += delta.x * m_RotationSpeed;
	m_Pitch += delta.y * m_RotationSpeed;
}

void OrbitCamera::Zoom(float delta) {
	m_Distance *= (1.0f + delta * m_ZoomSpeed);
	m_Distance = glm::max(m_Distance, 0.1f);
}

void OrbitCamera::Pan(const glm::vec2& delta) {
	glm::vec3 rightDirection = glm::normalize(glm::cross(m_Target - m_Position, m_Up));
	
	m_Target += -rightDirection * delta.x * m_PanSpeed * m_Distance;
	m_Target += m_Up * delta.y * m_PanSpeed * m_Distance;
}

//
//	FirstPersonCamera
//

}