#pragma once
#include "Scene.h"

#include <glm/vec3.hpp>

#include <optional>

struct HitRecord
{
	float hitDistance;
	glm::vec3 normal;
};


struct Ray
{
	glm::vec3 origin;
	glm::vec3 direction;
};

const glm::vec3 Trace(const Scene& scene, const Ray& ray) noexcept;
const glm::vec3 PointAlongRay(const Ray& ray, float distance) noexcept;
const std::optional<HitRecord> ClosestIntersection(const Scene& scene, const Ray& ray) noexcept;