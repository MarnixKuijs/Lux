#pragma once

#include "Mesh.h"
#include "Material.h"

struct Object
{
	const Mesh* geometry;
	const Material* material;
};