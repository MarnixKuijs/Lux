#pragma once
#include <glm/vec3.hpp>

#include <gsl/span>

#include <vector>

struct Mesh
{
	std::vector<glm::vec3> posistions;
	std::vector<glm::vec3> normals;
};