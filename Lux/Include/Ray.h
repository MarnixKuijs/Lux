#pragma once
#include "HitRecord.h"

#include <glm/vec3.hpp>
#include <glm/vec2.hpp>

#include <gsl/span>

#include <array>
#include <optional>

struct Scene;

struct Ray
{
	glm::vec3 origin;
	glm::vec3 direction;
};

glm::vec3 Trace(const Scene& scene, const Ray& ray) noexcept;
glm::vec3 PointAlongRay(const Ray& ray, float distance) noexcept;
std::optional<HitRecord> ClosestIntersection(const Scene& scene, const Ray& ray) noexcept;
glm::vec3 DirectIllumination(const Scene& scene, glm::vec3 hitPoint, glm::vec3 normal) noexcept;
bool IsOccluded(const Scene& scene, const Ray& lightRay, float distance) noexcept;
glm::vec3 Reflect(glm::vec3 incoming, glm::vec3 normal);
bool Intersection(const Ray& ray, glm::vec3 min, glm::vec3 max);
std::optional<TriangleHitRecord> Intersection(const Ray& ray, gsl::span<const glm::vec3, 3> triangle);