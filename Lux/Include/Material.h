#pragma once
#include <Color.h>
#include <glm/vec3.hpp>

#include "Texture.h"

struct Material
{
	glm::vec3 albedoColor{ Color::white };
	const Texture* albedoTexture;
};
