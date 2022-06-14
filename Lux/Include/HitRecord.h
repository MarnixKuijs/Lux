#pragma once 
#include <glm/vec3.hpp>
#include <glm/vec2.hpp>

#include <gsl/span>

#include <array>

struct Material;
struct Mesh;

struct HitRecord
{
	float hitDistance;
	//glm::vec3 faceNormal;
	glm::vec2 barycentric;
	size_t triangleIndex;
	const Material* material;
	const Mesh* mesh;
};

struct BvhHitRecord
{
	float hitDistance;
	size_t triangleIndex;
	glm::vec2 barycentric;
};

struct TriangleHitRecord
{
	float hitDistance;
	glm::vec2 barycentric;
};
