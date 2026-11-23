#include "component/CameraComponent.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/constants.hpp>

void CameraComponent::UpdateMatrices(int screenWidth, int screenHeight)
{
	float aspectRatio = static_cast<float>(screenWidth) / screenHeight;

	float pitchRad = glm::radians(pitch);
	float yawRad = glm::radians(yaw);
	glm::vec3 lookDir = glm::normalize(glm::vec3(
		cos(pitchRad) * sin(yawRad),  // x - east component
		cos(pitchRad) * cos(yawRad),  // y - north component
		sin(pitchRad)                 // z - up/down component
	));

	glm::vec3 upVec = { 0.0f, 0.0f, 1.0f };

	viewMatrix = glm::lookAt(pos, pos + lookDir, upVec);
	projMatrix =
		glm::perspective(
		glm::radians(fov),
		aspectRatio,
		1.0f, // near plane: 1 meter (avoids z-fighting right at ground level)
		50000.0f // far plane: 50 km (covers the full dataset)
	);

	viewProjMatrix = projMatrix * viewMatrix;
}

glm::vec3 CameraComponent::GetForward() const
{
	float pitchRad = glm::radians(pitch);
	float yawRad = glm::radians(yaw);

	return glm::normalize(glm::vec3(
		cos(pitchRad) * sin(yawRad),
		cos(pitchRad) * cos(yawRad),
		sin(pitchRad)
	));
}

glm::vec3 CameraComponent::GetRight() const
{
	glm::vec3 forward = GetForward();
	glm::vec3 up = { 0.0f, 0.0f, 1.0f };
	return glm::normalize(glm::cross(forward, up));
}
