#pragma once
#include <glm/vec3.hpp>
#include <glm/vec2.hpp>

#include <vector>

struct Mesh
{
	std::vector<glm::vec3> posistions;
	std::vector<glm::vec3> normals;
	std::vector<glm::vec2> texCoords;
};