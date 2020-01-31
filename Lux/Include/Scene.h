#pragma once
#include "Mesh.h"
#include "Material.h"
#include "Light.h"

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
	Material* material;
};

struct Scene
{
	Scene();
	std::vector<PointLight> lights;
	std::vector<Object> objects;
};