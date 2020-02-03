#pragma once
#include "Mesh.h"

#include <fx/gltf.h>

#include <cstdint>
#include <string>
#include <string_view>
#include <memory>
#include <filesystem>
#include <vector>

template <typename T>
struct Resource
{
	using Type = T;
	
	std::unique_ptr<T> value;
	uint32_t id;
	std::string name;
};

class ResourceManager
{
public:
	void ImportFromGltf(std::filesystem::path&& filePath);

	const Mesh& GetMeshByIndex(size_t index);
	const Mesh& GetMeshByResourceID(uint32_t id);
	const Mesh& GetMeshByName(std::string_view name);

private:
	void ParseNode(const fx::gltf::Document& gltf, const fx::gltf::Node& node);
	void ConvertMesh(const fx::gltf::Document& gltf, const fx::gltf::Mesh gltfMesh);
	std::vector<Resource<Mesh>> meshes;
};