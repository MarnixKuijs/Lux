#include "Ray.h"
#include "Color.h"

#include <glm/vec3.hpp>
#include <glm/geometric.hpp>

#include <algorithm>
#include <limits>

constexpr static float epsilon = 0.0000001f;

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
			return value.material->albedoColor /** Trace(scene, Ray{ hitPoint, Reflect(ray.direction, value.normal) })*/;
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
	size_t closestTriangle{ std::numeric_limits<size_t>::max() };

	for (size_t objectIndex{ 0 }; objectIndex < scene.objects.size(); ++objectIndex)
	{
		const Object& object{ scene.objects[objectIndex] };
		for (size_t vertexIndex{ 0 }; vertexIndex < object.geometry->posistions.size(); vertexIndex += 3)
		{
			glm::vec3 vertex0 = object.geometry->posistions[vertexIndex];
			glm::vec3 vertex1 = object.geometry->posistions[vertexIndex + 1];
			glm::vec3 vertex2 = object.geometry->posistions[vertexIndex + 2];
			glm::vec3 edge1 = vertex1 - vertex0;
			glm::vec3 edge2 = vertex2 - vertex0;
			glm::vec3 pvec = glm::cross(ray.direction, edge2);
			float det = glm::dot(edge1, pvec);

			if (det > -epsilon && det < epsilon)
			{
				continue;
			}

			float invDet = 1.0f / det;
			glm::vec3 tvec = ray.origin - vertex0;
			float u = invDet * glm::dot(tvec, pvec);

			if (u < 0.0f || u > 1.0f)
			{
				continue;
			}

			glm::vec3 qvec = glm::cross(tvec, edge1);
			float v = invDet * glm::dot(ray.direction, qvec);

			if (v < 0.0f || u + v > 1.0f)
			{
				continue;
			}

			float t = invDet * glm::dot(edge2, qvec);
			if (t > epsilon && t < 1.0f / epsilon && t < closestHitDistance)
			{
				closestHitDistance = t;
				closestObjectIndex = objectIndex;
				closestTriangle = vertexIndex;
			}
		}
	}

	if (closestHitDistance < std::numeric_limits<float>::max())
	{
		const Object& closestObject = scene.objects[closestObjectIndex];
		glm::vec3 closestVertex0 = closestObject.geometry->posistions[closestTriangle];
		glm::vec3 closestVertex1 = closestObject.geometry->posistions[closestTriangle + 1];
		glm::vec3 closestVertex2 = closestObject.geometry->posistions[closestTriangle + 2];
		glm::vec3 A = closestVertex1 - closestVertex0;
		glm::vec3 B = closestVertex2 - closestVertex0;
		glm::vec3 normal = glm::cross(A, B);
		float temp = glm::dot(ray.direction, normal);
		if (temp > 0.0f)
		{
			normal = -normal;
		}
		return HitRecord{closestHitDistance, glm::normalize(normal), closestObject.material };
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
			color += light.color * glm::dot(normal, lightDirNormalized) / lengthSquared;
		}
	}

	return color;
}

const bool IsOccluded(const Scene& scene, glm::vec3 hitPoint, glm::vec3 lightDirection, float distance) noexcept
{
	for (size_t objectIndex{ 0 }; objectIndex < scene.objects.size(); ++objectIndex)
	{
		const Object& object{ scene.objects[objectIndex] };
		for (size_t vertexIndex{ 0 }; vertexIndex < object.geometry->posistions.size(); vertexIndex += 3)
		{
			glm::vec3 vertex0 = object.geometry->posistions[vertexIndex];
			glm::vec3 vertex1 = object.geometry->posistions[vertexIndex + 1];
			glm::vec3 vertex2 = object.geometry->posistions[vertexIndex + 2];
			glm::vec3 edge1 = vertex1 - vertex0;
			glm::vec3 edge2 = vertex2 - vertex0;
			glm::vec3 pvec = glm::cross(lightDirection, edge2);
			float det = glm::dot(edge1, pvec);

			if (det > -epsilon && det < epsilon)
			{
				continue;
			}

			float invDet = 1.0f / det;
			glm::vec3 tvec = hitPoint - vertex0;
			float u = invDet * glm::dot(tvec, pvec);

			if (u < 0.0f || u > 1.0f)
			{
				continue;
			}

			glm::vec3 qvec = glm::cross(tvec, edge1);
			float v = invDet * glm::dot(lightDirection, qvec);

			if (v < 0.0f || u + v > 1.0f)
			{
				continue;
			}

			float t = invDet * glm::dot(edge2, qvec);
			if (t > epsilon && t < 1.0f / epsilon && t < distance)
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

