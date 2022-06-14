#include "Ray.h"
#include "Color.h"
#include "Scene.h"
#include "Material.h"
#include "Mesh.h"
#include "Bvh.h"

#include <glm/vec3.hpp>
#include <glm/geometric.hpp>

#include <algorithm>
#include <limits>
#include <utility>

constexpr static float epsilon = 0.0000001f;
constexpr static float epsilonShadowAcne = 0.00001f;

glm::vec3 Trace(const Scene& scene, const Ray& ray) noexcept
{
	auto hitRecord = ClosestIntersection(scene, ray);
	if (hitRecord)
	{
		const HitRecord& value = hitRecord.value();
		const glm::vec3 hitPoint = PointAlongRay(ray, value.hitDistance);

		const glm::vec3 closestVertex0 = value.mesh->posistions[value.triangleIndex];
		const glm::vec3 closestVertex1 = value.mesh->posistions[value.triangleIndex + 1];
		const glm::vec3 closestVertex2 = value.mesh->posistions[value.triangleIndex + 2];

		glm::vec3 normal = glm::cross(closestVertex1 - closestVertex0, closestVertex2 - closestVertex0);
		float sign = glm::dot(ray.direction, normal);
		normal *= float(-((sign > 0.0f) - (sign < 0.0f)));

		const glm::vec2 texCoord0 = value.mesh->texCoords[value.triangleIndex] * (1.0f - value.barycentric.x - value.barycentric.y);
		const glm::vec2 texCoord1 = value.mesh->texCoords[value.triangleIndex + 1] * value.barycentric.x;
		const glm::vec2 texCoord2 = value.mesh->texCoords[value.triangleIndex + 2] * value.barycentric.y;
		const glm::vec2 uv{ texCoord0 + texCoord1 + texCoord2 };

		return value.material->albedoColor * SampleTexture(*value.material->albedoTexture, uv) /** DirectIllumination(scene, hitPoint, normal)*/;

	}
	else
	{
		return glm::vec3{ 0.5f, 0.5f, 1.0f };
	}
}

glm::vec3 PointAlongRay(const Ray& ray, float distance) noexcept
{
	return ray.origin + distance * ray.direction;
}

std::optional<HitRecord> ClosestIntersection(const Scene& scene, const Ray& ray) noexcept
{
	float closestHitDistance{ std::numeric_limits<float>::max() };
	size_t closestObjectIndex{ std::numeric_limits<size_t>::max() };
	glm::vec2 closestBarycentric{ std::numeric_limits<float>::max(), std::numeric_limits<float>::max() };

	size_t closestTriangle{ std::numeric_limits<size_t>::max() };


	for (size_t objectIndex{ 0 }; objectIndex < scene.objects.size(); ++objectIndex)
	{
		
		const Object& object{ scene.objects[objectIndex] };
		auto  traverseHit = object.bvh->Traverse(object.bvh->Root(), ray, object.geometry->posistions);

		if (!traverseHit)
		{
			continue;
		}

		const BvhHitRecord& bvhHitRecord = traverseHit.value();

		if (bvhHitRecord.hitDistance < closestHitDistance)
		{
			closestHitDistance = bvhHitRecord.hitDistance;
			closestObjectIndex = objectIndex;
			closestTriangle = bvhHitRecord.triangleIndex;
			closestBarycentric = bvhHitRecord.barycentric;
		}
	}

	if (closestHitDistance < std::numeric_limits<float>::max())
	{
		const Object& closestObject = scene.objects[closestObjectIndex];

		return HitRecord
		{
			closestHitDistance,
			closestBarycentric,
			closestTriangle,
			closestObject.material,
			closestObject.geometry
		};
	}
	else
	{
		return std::nullopt;
	}

}

glm::vec3 DirectIllumination(const Scene& scene, glm::vec3 hitPoint, glm::vec3 normal) noexcept
{
	glm::vec3 color = Color::black;
	auto& lights = scene.lights;
	for (auto& light : lights)
	{
		glm::vec3 lightDirection = light.position - hitPoint;
		float lengthSquared = glm::dot(lightDirection, lightDirection);
		float length = sqrtf(lengthSquared);

		if (!IsOccluded(scene, Ray{ hitPoint + lightDirection * epsilonShadowAcne, lightDirection / length }, length))
		{
			glm::vec3 lightDirNormalized = lightDirection / length;
			color += light.color * glm::dot(normal, lightDirNormalized) / lengthSquared;
		}
	}

	return color;
}

bool IsOccluded(const Scene& scene, const Ray& lightRay, float distance) noexcept
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
			glm::vec3 pvec = glm::cross(lightRay.direction, edge2);
			float det = glm::dot(edge1, pvec);

			if (det > -epsilon && det < epsilon)
			{
				continue;
			}

			float invDet = 1.0f / det;
			glm::vec3 tvec = lightRay.origin - vertex0;
			float u = invDet * glm::dot(tvec, pvec);

			if (u < 0.0f || u > 1.0f)
			{
				continue;
			}

			glm::vec3 qvec = glm::cross(tvec, edge1);
			float v = invDet * glm::dot(lightRay.direction, qvec);

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

glm::vec3 Reflect(glm::vec3 incoming, glm::vec3 normal)
{
	return incoming - 2 * glm::dot(incoming, normal) * normal;
}

bool Intersection(const Ray& ray, glm::vec3 min, glm::vec3 max)
{

	float t1 = (min.x - ray.origin.x) / ray.direction.x;
	float t2 = (max.x - ray.origin.x) / ray.direction.x;
	float t3 = (min.y - ray.origin.y) / ray.direction.y;
	float t4 = (max.y - ray.origin.y) / ray.direction.y;
	float t5 = (min.z - ray.origin.z) / ray.direction.z;
	float t6 = (max.z - ray.origin.z) / ray.direction.z;

	float tmin = std::max(std::max(std::min(t1, t2), std::min(t3, t4)), std::min(t5, t6));
	float tmax = std::min(std::min(std::max(t1, t2), std::max(t3, t4)), std::max(t5, t6));

	if (tmax < 0)
	{
		return false;
	}

	if (tmin > tmax)
	{
		return false;
	}

	return true;
}

std::optional<TriangleHitRecord> Intersection(const Ray& ray, gsl::span<const glm::vec3, gsl::dynamic_extent> triangle)
{
	glm::vec3 vertex0 = triangle[0];
	glm::vec3 vertex1 = triangle[1];;
	glm::vec3 vertex2 = triangle[2];;
	glm::vec3 edge1 = vertex1 - vertex0;
	glm::vec3 edge2 = vertex2 - vertex0;
	glm::vec3 pvec = glm::cross(ray.direction, edge2);
	float det = glm::dot(edge1, pvec);

	if (det > -epsilon && det < epsilon)
	{
		return std::nullopt;
	}

	float invDet = 1.0f / det;
	glm::vec3 tvec = ray.origin - vertex0;
	float u = invDet * glm::dot(tvec, pvec);

	if (u < 0.0f || u > 1.0f)
	{
		return std::nullopt;
	}

	glm::vec3 qvec = glm::cross(tvec, edge1);
	float v = invDet * glm::dot(ray.direction, qvec);

	if (v < 0.0f || u + v > 1.0f)
	{
		return std::nullopt;
	}

	float t = invDet * glm::dot(edge2, qvec);
	if (t > epsilon && t < 1.0f / epsilon)
	{
		return TriangleHitRecord{ t, { u, v } };
	}

	return std::nullopt;
}

