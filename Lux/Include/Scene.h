#pragma once
#include "Light.h"
#include "Object.h"
#include <glm/vec3.hpp>

#include <vector>

struct Scene
{
	std::vector<PointLight> lights;
	std::vector<Object> objects;
};