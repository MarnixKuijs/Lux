#pragma once

#include <glm/vec3.hpp>

struct Camera
{
	Camera(glm::vec3 position, glm::vec3 lookat, float vfov, float aspect);

    glm::vec3 position;
    glm::vec3 lower_left_corner;
    glm::vec3 horizontal;
    glm::vec3 vertical;
};