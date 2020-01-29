#include "Ray.h"

#include <glm/geometric.hpp>

#include <limits>

const glm::vec3 Trace(const Scene& scene, const Ray& ray) noexcept
{
	auto hitRecord = ClosestIntersection(scene, ray);
	if (hitRecord)
	{
		return glm::vec3{ 1.0f, 1.0f, 1.0f };
	}
	else
	{
		return glm::vec3{ 0.0f, 0.0f, 0.0f };
	}
}

const glm::vec3 PointAlongRay(const Ray& ray, float distance) noexcept
{
	return glm::vec3();
}

const std::optional<HitRecord> ClosestIntersection(const Scene& scene, const Ray& ray) noexcept
{
	float closestHitDistance{ std::numeric_limits<float>::max() };
	size_t closestObjectIndex{ std::numeric_limits<size_t>::max() };

	for (size_t i{ 0 }; i < scene.objects.size(); ++i)
	{
		const Object& object{ scene.objects[i] };

		glm::vec3 C = object.geometry.center - ray.origin ;
		float t = glm::dot(C, ray.direction);
		glm::vec3 Q = C - t * ray.direction;
		float p2 = glm::dot(Q, Q);
		
		float r2 = object.geometry.radius * object.geometry.radius;
		if (p2 > r2)
		{
			continue;
		}

		t -= std::sqrtf(r2);

		if (t < closestHitDistance && t > 0)
		{
			closestHitDistance = t;
			closestObjectIndex = i;
		}
	}

	if (closestHitDistance != std::numeric_limits<float>::max())
	{
		const Object& closestObject = scene.objects[closestObjectIndex];
		return HitRecord{closestHitDistance, (PointAlongRay(ray, closestHitDistance) - closestObject.geometry.center) / closestObject.geometry.radius };
	}
	else
	{
		return std::nullopt;
	}

}
