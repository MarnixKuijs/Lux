#pragma 

struct Mesh;
struct Material;
class Bvh;

struct Object
{
	const Mesh* geometry;
	const Material* material;
	const Bvh* bvh;
};