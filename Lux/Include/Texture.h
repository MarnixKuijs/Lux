#pragma once
#include <glm/vec3.hpp>
#include <glm/vec2.hpp>

#include <cstdint>
#include <vector>

struct Texture
{
	uint32_t width;
	uint32_t height;
	uint32_t channels;
	std::vector<uint8_t> data;
};

glm::vec3 SampleTexture(const Texture& texture, glm::vec2 texCoords);