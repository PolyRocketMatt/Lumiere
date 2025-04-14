#pragma once

#include "glm/glm.hpp"

#include <vector>

namespace Lumiere {

struct CameraMetadata {
	//	--- Geometry & Optics ---
	glm::vec3	s_Position;
	glm::vec3	s_Direction;
	glm::vec3	s_Up;

	float		s_VerticalFov;		
	float		s_FocalLength;		
	float		s_Aperture;
	float		s_FocusDistance;
	float		s_NearClip;
	float		s_FarClip;

	//	--- Sensor ---
	float		s_SensorWidth;
	float		s_SensorHeight;
	float		s_SensorOpenTime;
	float		s_SensorCloseTime;

	//	--- Misc ---
	uint32_t	s_ImageWidth;
	uint32_t	s_ImageHeight;
	float		s_AspectRatio;

	//  --- Real-Time ---
	bool		s_ContinuousRender;
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
	
	const glm::vec3 GetPosition() const { return m_Position; }
	const glm::vec3 GetDirection() const { return m_Direction; }
	const glm::vec3 GetUp() const { return m_Up; }

	const std::vector<glm::vec3>& GetRayDirections() const { return m_RayDirections; }
private:
	void RecalculateProjection();
	void RecalculateView();
	void RecalculateRayDirections();
	
private:
	glm::mat4				m_Projection{ 1.0f };
	glm::mat4				m_View{ 1.0f };
	glm::mat4				m_InverseProjection{ 1.0f };
	glm::mat4				m_InverseView{ 1.0f };

	glm::vec3				m_Position;
	glm::vec3				m_Direction;
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

}