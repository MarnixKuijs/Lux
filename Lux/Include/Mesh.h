#pragma once
#include <glm/vec3.hpp>

#include <gsl/span>

#include <vector>

struct Mesh
{
	Mesh();
	std::vector<glm::vec3> triangles;
};