#include "Camera.h"
#include <cmath>
#include <glm/geometric.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtc/matrix_transform.hpp>

Camera::Camera(glm::vec3 position, glm::vec3 lookat, float vfov, float aspect)
    : position(position)
{
	float theta = vfov * glm::pi<float>() / 180.0f;
    float halfHeight = std::tan(theta / 2);
    float halfWidth = aspect * halfHeight;
    
    glm::vec3 cameraDirection = glm::normalize(position - lookat);
    glm::vec3 cameraRight = glm::normalize(glm::cross(glm::vec3{ 0.0f, 1.0f, 0.0f }, cameraDirection));
    glm::vec3 cameraUp = glm::cross(cameraDirection, cameraRight);

    lower_left_corner = position - halfWidth * cameraRight - halfHeight * cameraUp - cameraDirection;
    horizontal = 2 * halfWidth * cameraRight;
    vertical = 2 * halfHeight * cameraUp;
}