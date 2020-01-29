#pragma once
#include <glm/vec3.hpp>

#include <vector>

//Temp
struct Sphere
{
	glm::vec3 center;
	float radius;
};

struct Object
{
	Sphere geometry;
};

struct Scene
{
	std::vector<Object> objects;
};