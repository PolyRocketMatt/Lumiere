#include "Camera.h"

#include "glm/gtc/matrix_transform.hpp";
#include "glm/gtc/quaternion.hpp"
#include "glm/gtx/quaternion.hpp"

namespace Lumiere {

BaseCamera::BaseCamera(const CameraMetadata& metaData)
	: m_Position(metaData.s_Position),
	m_Direction(metaData.s_Direction),
	m_Up(metaData.s_Up),
	m_VerticalFov(metaData.s_VerticalFov),
	m_NearClip(metaData.s_NearClip),
	m_FarClip(metaData.s_FarClip),
	m_ImageWidth(metaData.s_ImageWidth),
	m_ImageHeight(metaData.s_ImageHeight),
	m_ContinuousRender(metaData.s_ContinuousRender)
{
	m_AspectRatio = (float)m_ImageWidth / m_ImageHeight;
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

			//	Calculate ray direction from origin
			glm::vec4 target = m_InverseProjection * glm::vec4(coord.x, coord.y, 1, 1);
			glm::vec3 rayDirection = glm::vec3(m_InverseView * glm::vec4(glm::normalize(glm::vec3(target) / target.w), 0.0f));
			
			m_RayDirections[x + y * m_ImageWidth] = rayDirection;
		}
	}
}

}