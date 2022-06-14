#pragma once
#include "Mesh.h"
#include "Texture.h"
#include "Material.h"

#include <fx/gltf.h>

#include <cstdint>
#include <string>
#include <string_view>
#include <memory>
#include <filesystem>
#include <vector>

struct ImportSettings
{
	bool flipUV = false;
};

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
	ResourceManager();
	void ImportFromGltf(std::filesystem::path&& filePath, ImportSettings settings);
	const Texture& LoadTexture(std::filesystem::path&& filePath);

	const Mesh& GetMeshByIndex(size_t index);
	const Mesh& GetMeshByResourceID(uint32_t id);
	const Mesh& GetMeshByName(std::string_view name);

	const Material& GetMaterialByIndex(size_t index);

	const Texture& DefaultAlbedo() { return *defaultAlbedo.value; }
	const Material& DefaultMaterial() { return *defaultMaterial.value; }

private:
	void LoadDefaults();
	void ParseNode(const fx::gltf::Document& gltf, const fx::gltf::Node& node, ImportSettings settings);
	void ConvertMesh(const fx::gltf::Document& gltf, const fx::gltf::Mesh& gltfMesh, ImportSettings settings);
	void ConvertMaterial(const fx::gltf::Document& gltf, const fx::gltf::Material& gltfMaterial);
	std::vector<Resource<Mesh>> meshes;
	std::vector<Resource<Texture>> textures;
	std::vector<Resource<Material>> materials;

	std::filesystem::path* currentAssetPath = nullptr;

	Resource<Texture> defaultAlbedo;
	Resource<Material> defaultMaterial;
};