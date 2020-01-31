#pragma once
#include "Scene.h"
#include "Light.h"

#include <glm/vec3.hpp>
#include <gsl/span>

#include <optional>

struct HitRecord
{
	float hitDistance;
	glm::vec3 normal;
	Material* material;
};


struct Ray
{
	glm::vec3 origin;
	glm::vec3 direction;
};

const glm::vec3 Trace(const Scene& scene, const Ray& ray) noexcept;
const glm::vec3 PointAlongRay(const Ray& ray, float distance) noexcept;
const std::optional<HitRecord> ClosestIntersection(const Scene& scene, const Ray& ray) noexcept;
const glm::vec3 DirectIllumination(const Scene& scene, glm::vec3 hitPoint, glm::vec3 normal) noexcept;
const bool IsOccluded(const Scene& scene, glm::vec3 hitPoint, glm::vec3 lightDirection, float distance) noexcept;
const glm::vec3 Reflect(glm::vec3 incoming, glm::vec3 normal);
