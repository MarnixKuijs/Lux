#pragma once
#include "HitRecord.h"

#include <glm/vec3.hpp>

#include <gsl/span>

#include <vector>
#include <optional>

struct Mesh;
struct Ray;

struct BvhNode
{
	glm::vec3 min;
	uint32_t leftFirst;
	glm::vec3 max;
	uint32_t count;
};

class Bvh
{
public:
	Bvh(const Mesh& mesh);
	std::optional<BvhHitRecord> Traverse(const BvhNode& bvhNode, const Ray& ray, gsl::span<const glm::vec3, gsl::dynamic_extent> triangles) const noexcept;
	const BvhNode& Root() const noexcept { return *root; }
private:
	void SubDevide(BvhNode& bvhNode, gsl::span<const glm::vec3, gsl::dynamic_extent> positions, gsl::span<const glm::vec3, gsl::dynamic_extent> centroids);
	void CalculateBounds(BvhNode& node, gsl::span<const glm::vec3, gsl::dynamic_extent> positions);

	BvhNode* root;
	uint64_t poolPtr;
	std::vector<uint32_t> indices;
	std::vector<BvhNode> pool;
};

