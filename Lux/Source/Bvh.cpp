#include "Bvh.h"
#include "Mesh.h"
#include "Ray.h"
#include <glm/vec3.hpp>

#include <gsl/span>

#include <numeric>
#include <algorithm>
#include <tuple>
#include <limits>


void Bvh::CalculateBounds(BvhNode& node, gsl::span<const glm::vec3, gsl::dynamic_extent> positions)
{
	glm::vec3 min{ std::numeric_limits<float>::max() };
	glm::vec3 max{ std::numeric_limits<float>::min() };

	for (uint32_t i = 0; i < node.count; ++i)
	{
		uint32_t triangleIndex = indices[node.leftFirst + i];

		for (uint32_t j = 0; j < 3; ++j)
		{
			glm::vec3 vertex = positions[triangleIndex * 3 + j];

			for (uint32_t k = 0; k < 3; ++k)
			{
				if (vertex[k] < min[k])
				{
					min[k] = vertex[k];
				}

				if (vertex[k] > max[k])
				{
					max[k] = vertex[k];
				}
			}
		}
	}

	node.min = min;
	node.max = max;

}

void CalculateCentroids(gsl::span<const glm::vec3, gsl::dynamic_extent> positions, std::vector<glm::vec3>& centroids)
{
	centroids.reserve(positions.size() / 3);
	for (size_t triangleIndex{ 0 }; triangleIndex < static_cast<size_t>(positions.size()); triangleIndex += 3)
	{
		centroids.push_back((positions[triangleIndex] + positions[triangleIndex + 1] + positions[triangleIndex + 2]) / 3.0f);
	}
}

Bvh::Bvh(const Mesh& mesh)
{
	size_t numTriangles = mesh.posistions.size() / static_cast<size_t>(3);

	indices.resize(numTriangles);
	std::iota(indices.begin(), indices.end(), 0);

	pool.resize(numTriangles * 2);

	root = &pool[0];
	poolPtr = 2;

	root->leftFirst = 0;
	root->count = static_cast<uint32_t>(numTriangles);

	auto positions = gsl::make_span(mesh.posistions.data() + static_cast<uint64_t>(root->leftFirst) * 3, mesh.posistions.data() + static_cast<uint64_t>(root->count) * 3);
	std::vector<glm::vec3> centroids;
	CalculateCentroids(positions, centroids);
	CalculateBounds(*root, positions);


	SubDevide(*root, positions, centroids);

}

void Bvh::SubDevide(BvhNode& bvhNode, gsl::span<const glm::vec3, gsl::dynamic_extent> positions, gsl::span<const glm::vec3, gsl::dynamic_extent> centroids)
{
	if (bvhNode.count < 3)
	{
		return;
	}

	glm::vec3 splitPoint = bvhNode.min + (bvhNode.max - bvhNode.min) / 2.0f;
	auto startingItter = indices.begin() + bvhNode.leftFirst;
	auto& itter = std::partition(startingItter, startingItter + bvhNode.count, [centroids, splitPoint](const auto& lhs)
		{
			return centroids[lhs][0] < splitPoint[0];
		});

	uint32_t leftCount = static_cast<uint32_t>(std::distance(indices.begin() + bvhNode.leftFirst, itter));
	uint32_t rightCount = static_cast<uint32_t>(std::distance(itter, indices.begin() + bvhNode.leftFirst + bvhNode.count));
	uint32_t leftBegin = bvhNode.leftFirst;
	uint32_t rightBegin = static_cast<uint32_t>(itter - indices.begin());

	bvhNode.count = 0;
	bvhNode.leftFirst = static_cast<uint32_t>(poolPtr);

	BvhNode& leftNode = pool[poolPtr++];

	leftNode.count = leftCount;
	leftNode.leftFirst = leftBegin;

	CalculateBounds(leftNode, positions);

	BvhNode& rightNode = pool[poolPtr++];

	rightNode.count = rightCount;
	rightNode.leftFirst = rightBegin;

	CalculateBounds(rightNode, positions);

	if ((leftNode.min == bvhNode.min && leftNode.max == bvhNode.max) || (rightNode.min == bvhNode.min && rightNode.max == bvhNode.max))
	{
		return;
	}

	SubDevide(leftNode, positions, centroids);
	SubDevide(rightNode, positions, centroids);

}

std::optional<BvhHitRecord> Bvh::Traverse(const BvhNode& bvhNode, const Ray& ray, gsl::span<const glm::vec3, gsl::dynamic_extent> triangles) const noexcept
{
	if (!Intersection(ray, bvhNode.min, bvhNode.max))
	{
		return std::nullopt;
	}

	if (bvhNode.count > 0)
	{
		float closestHitDistance{ std::numeric_limits<float>::max() };
		glm::vec2 closestBarycentric{ std::numeric_limits<float>::max(), std::numeric_limits<float>::max() };
		size_t closestTriangleIndex{ std::numeric_limits<size_t>::max() };

		for (size_t triangleIndex = 0; triangleIndex < bvhNode.count; ++triangleIndex)
		{
			auto triangleHit = Intersection(ray, gsl::make_span(&triangles[(bvhNode.leftFirst + triangleIndex) * 3], 3));
			if (!triangleHit)
			{
				continue;
			}

			const TriangleHitRecord& triangleHitRecord = triangleHit.value();

			if (triangleHitRecord.hitDistance < closestHitDistance)
			{
				closestHitDistance = triangleHitRecord.hitDistance;
				closestBarycentric = triangleHitRecord.barycentric;
				closestTriangleIndex = (bvhNode.leftFirst + triangleIndex) * 3;
			}
		}

		if (closestHitDistance < std::numeric_limits<float>::max())
		{
			return BvhHitRecord
			{
				closestHitDistance,
				closestTriangleIndex,
				closestBarycentric,
			};
		}
		else
		{
			return std::nullopt;
		}
	}
	else
	{
		std::optional<BvhHitRecord> leftValue = Traverse(pool[bvhNode.leftFirst], ray, triangles);
		std::optional<BvhHitRecord> rightValue = Traverse(pool[bvhNode.leftFirst + 1], ray, triangles);


		if (leftValue && rightValue)
		{
			if (leftValue.value().hitDistance < rightValue.value().hitDistance)
			{
				return leftValue;
			}
			else
			{
				return rightValue;
			}
		}
		else if (leftValue)
		{
			return leftValue;
		}
		else if (rightValue)
		{
			return rightValue;
		}
	}

	return std::nullopt;
}
