#pragma once

#include <glm/vec3.hpp>

struct Color
{
	constexpr static glm::vec3 black{ 0.0f, 0.0f, 0.0f };
	constexpr static glm::vec3 white{ 1.0f, 1.0f, 1.0f };
	constexpr static glm::vec3 red{ 1.0f, 0.0f, 0.0f };
	constexpr static glm::vec3 green{ 0.0f, 1.0f, 0.0f };
	constexpr static glm::vec3 blue{ 0.0f, 0.0f, 1.0f };
};