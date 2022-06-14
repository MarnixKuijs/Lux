#include "Texture.h"

#include <glm/vec3.hpp>
#include <glm/vec2.hpp>

#include <algorithm>
#include <limits>
#include <cmath>

glm::vec3 SampleTexture(const Texture& texture, glm::vec2 texCoords)
{
	float u = std::clamp(texCoords.x, 0.0f, 1.0f - std::numeric_limits<float>::epsilon());
	float v = std::clamp(texCoords.y, 0.0f, 1.0f - std::numeric_limits<float>::epsilon());

	uint32_t x = std::min(static_cast<uint32_t>(std::floor(u * texture.width)), texture.width - 1);
	uint32_t y = std::min(static_cast<uint32_t>(std::floor(v * texture.height)), texture.height - 1);

	uint32_t pixelIndex = (x + y * texture.width) * texture.channels;

	return glm::vec3
	{
		static_cast<float>(texture.data[pixelIndex]) / 255.0f,
		static_cast<float>(texture.data[pixelIndex + 1]) / 255.0f,
		static_cast<float>(texture.data[pixelIndex + 2]) / 255.0f
	};
}