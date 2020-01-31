#include "Ray.h"
#include "Color.h"

#include <glm/vec3.hpp>
#include <glm/geometric.hpp>

#include <algorithm>
#include <limits>

const glm::vec3 Trace(const Scene& scene, const Ray& ray) noexcept
{
	auto hitRecord = ClosestIntersection(scene, ray);
	if (hitRecord)
	{
		HitRecord& value = hitRecord.value();
		glm::vec3 hitPoint = PointAlongRay(ray, value.hitDistance);

		if (value.material->metalicness == 0.0f)
		{
			return value.material->albedoColor * DirectIllumination(scene, hitPoint, value.normal);
		}
		else
		{
			return value.material->albedoColor * Trace(scene, Ray{ hitPoint, Reflect(ray.direction, value.normal) });
		}
	}
	else
	{
		return glm::vec3{ 0.5f, 0.5f, 1.0f };
	}
}

const glm::vec3 PointAlongRay(const Ray& ray, float distance) noexcept
{
	return ray.origin + distance * ray.direction;
}

const std::optional<HitRecord> ClosestIntersection(const Scene& scene, const Ray& ray) noexcept
{
	float closestHitDistance{ std::numeric_limits<float>::max() };
	size_t closestObjectIndex{ std::numeric_limits<size_t>::max() };

	for (size_t i{ 0 }; i < scene.objects.size(); ++i)
	{
		const Object& object{ scene.objects[i] };

		glm::vec3 hypotenuse = object.geometry.center - ray.origin;
		float t = glm::dot(hypotenuse, ray.direction);
		glm::vec3 opposite = hypotenuse - t * ray.direction;
		float lengthSquared = glm::dot(opposite, opposite);
		float radiusSquared = object.geometry.radius * object.geometry.radius;

		if (lengthSquared > radiusSquared || t < 0.0f)
		{
			continue;
		}

		float internalDistance = std::sqrtf(radiusSquared - lengthSquared);
		float hitDistance = (t - internalDistance) < 0.0f ? (t + internalDistance) : (t - internalDistance);

		if (hitDistance < closestHitDistance)
		{
			closestHitDistance = hitDistance;
			closestObjectIndex = i;
		}
	}

	if (closestHitDistance < std::numeric_limits<float>::max())
	{
		const Object& closestObject = scene.objects[closestObjectIndex];
		return HitRecord{closestHitDistance, glm::normalize((PointAlongRay(ray, closestHitDistance) - closestObject.geometry.center)), closestObject.material };
	}
	else
	{
		return std::nullopt;
	}

}

const glm::vec3 DirectIllumination(const Scene& scene, glm::vec3 hitPoint, glm::vec3 normal) noexcept
{
	glm::vec3 color = Color::black;
	auto& lights = scene.lights;
	for (auto& light : lights)
	{
		glm::vec3 lightDirection = light.position - hitPoint;
		float lengthSquared = glm::dot(lightDirection, lightDirection);
		float length = sqrtf(lengthSquared);

		if (!IsOccluded(scene, hitPoint, lightDirection, length))
		{
			glm::vec3 lightDirNormalized = lightDirection / length;
			color += light.color * std::max(0.0f, glm::dot(normal, lightDirNormalized)) / lengthSquared;
		}
	}

	return color;
}

const bool IsOccluded(const Scene& scene, glm::vec3 hitPoint, glm::vec3 lightDirection, float distance) noexcept
{
	glm::vec3 lightDirNormalized = lightDirection / distance;

	for (size_t i{ 0 }; i < scene.objects.size(); ++i)
	{
		const Object& object{ scene.objects[i] };

		glm::vec3 hypotenuse = object.geometry.center - hitPoint;
		float t = glm::dot(hypotenuse, lightDirNormalized);
		glm::vec3 opposite = hypotenuse - t * lightDirNormalized;
		float lengthSquared = glm::dot(opposite, opposite);
		float radiusSquared = object.geometry.radius * object.geometry.radius;

		if (t > 0.0f)
		{
			float internalDistance = std::sqrtf(radiusSquared - lengthSquared);
			float hitDistance = (t - internalDistance) < 0.0f ? (t + internalDistance) : (t - internalDistance);

			if (hitDistance < distance)
			{
				return true;
			}
		}
	}

	return false;
}

const glm::vec3 Reflect(glm::vec3 incoming, glm::vec3 normal)
{
	return incoming - 2 * glm::dot(incoming, normal) * normal;
}

