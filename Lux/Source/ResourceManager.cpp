#include "ResourceManager.h"
#include "Mesh.h"
#include "Texture.h"
#include "Material.h"
#include "stb_image.h"

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/geometric.hpp>

#include <fx/gltf.h>
#include <gsl/span>
#include <cstring>


ResourceManager::ResourceManager()
{
	LoadDefaults();
}

void ResourceManager::LoadDefaults()
{
	defaultAlbedo.value = std::make_unique<Texture>();
	int width, height, channels, desiredChannels = 3;
	uint8_t* imageData = stbi_load(ASSETDIR"/Textures/Default.png", &width, &height, &channels, desiredChannels);
	size_t imageDataSize = static_cast<size_t>(width)* static_cast<size_t>(height)* static_cast<size_t>(desiredChannels);
	defaultAlbedo.value->data.resize(imageDataSize);
	std::memcpy(defaultAlbedo.value->data.data(), imageData, imageDataSize);
	defaultAlbedo.value->width = width;
	defaultAlbedo.value->height = height;
	defaultAlbedo.value->channels = desiredChannels;
	defaultMaterial.value = std::make_unique<Material>();
	defaultMaterial.value->albedoTexture = defaultAlbedo.value.get();
}

const Texture& ResourceManager::LoadTexture(std::filesystem::path&& filePath)
{
	Resource<Texture>& textureResource = textures.emplace_back();
	textureResource.value = std::make_unique<Texture>();
	
	Texture& texture = *textureResource.value;
	

	int width, height, channels, desiredChannels = 3;
	uint8_t* imageData = stbi_load(filePath.string().c_str(), &width, &height, &channels, desiredChannels);
	size_t imageDataSize = static_cast<size_t>(width)* static_cast<size_t>(height)* static_cast<size_t>(desiredChannels);

	texture.data.resize(imageDataSize);
	std::memcpy(texture.data.data(), imageData, imageDataSize);

	texture.width = width;
	texture.height = height;
	texture.channels = desiredChannels;

	return texture;
}

void ResourceManager::ImportFromGltf(std::filesystem::path&& filePath, ImportSettings settings)
{
	currentAssetPath = &filePath;
	const fx::gltf::Document gltf = fx::gltf::LoadFromText(filePath.string());

	const fx::gltf::Scene& scene = gltf.scenes[gltf.scene];
	
	for (const uint32_t nodeIndex : scene.nodes)
	{		
		ParseNode(gltf, gltf.nodes[nodeIndex], settings);
	}
}

void ResourceManager::ParseNode(const fx::gltf::Document& gltf, const fx::gltf::Node& node, ImportSettings settings)
{
	if (node.mesh != -1)
	{
		ConvertMesh(gltf, gltf.meshes[node.mesh], settings);
	}

	for (const uint32_t nodeIndex : node.children)
	{
		ParseNode(gltf, gltf.nodes[nodeIndex], settings);
	}
}

void ResourceManager::ConvertMesh(const fx::gltf::Document& gltf, const fx::gltf::Mesh& gltfMesh, ImportSettings settings)
{
	Resource<Mesh>& meshResource = meshes.emplace_back();
	meshResource.value = std::make_unique<Mesh>();
	meshResource.name = gltfMesh.name;

	for (const fx::gltf::Primitive& primitve : gltfMesh.primitives)
	{
		const fx::gltf::Accessor& indicesAccessor = gltf.accessors[primitve.indices];
		const fx::gltf::BufferView& indicesBufferView = gltf.bufferViews[indicesAccessor.bufferView];
		const fx::gltf::Buffer& indicesBuffer = gltf.buffers[indicesBufferView.buffer];
		const uint8_t* indecesbufferViewStart = indicesBuffer.data.data() + indicesBufferView.byteOffset;
		
		gsl::span<const uint8_t, gsl::dynamic_extent> indicesData
		{
			indecesbufferViewStart + indicesAccessor.byteOffset,
			indecesbufferViewStart + indicesBufferView.byteLength
		};

		std::vector<uint16_t> indices{};
		indices.resize(static_cast<size_t>(indicesAccessor.count));

		for (uint32_t i{ 0 }; i < indicesAccessor.count; ++i)
		{
			uint32_t index = i * sizeof(uint16_t);

			std::memcpy(&(indices[i]), &(indicesData[index]), sizeof(uint16_t));
		}

		const fx::gltf::Accessor& positionAccessor = gltf.accessors[primitve.attributes.at("POSITION")];
		const fx::gltf::BufferView& positionBufferView = gltf.bufferViews[positionAccessor.bufferView];
		const fx::gltf::Buffer& positionBuffer = gltf.buffers[positionBufferView.buffer];
		const uint8_t* positionBufferViewStart = positionBuffer.data.data() + positionBufferView.byteOffset;

		gsl::span<const uint8_t, gsl::dynamic_extent> positionData
		{
			positionBufferViewStart + positionAccessor.byteOffset,
			positionBufferViewStart + positionBufferView.byteLength
		};

		std::vector<glm::vec3> positions{};
		positions.resize(static_cast<size_t>(positionAccessor.count));

		for (uint32_t positionIndex{ 0 }; positionIndex < positionAccessor.count; ++positionIndex)
		{
			uint32_t index = positionIndex * sizeof(glm::vec3);

			std::memcpy(&(positions[positionIndex]), &(positionData[index]), sizeof(glm::vec3));
		}

		const fx::gltf::Accessor& normalAccessor = gltf.accessors[primitve.attributes.at("NORMAL")];
		const fx::gltf::BufferView& normalBufferView = gltf.bufferViews[normalAccessor.bufferView];
		const fx::gltf::Buffer& normalBuffer = gltf.buffers[normalBufferView.buffer];
		const uint8_t* normalBufferViewStart = normalBuffer.data.data() + normalBufferView.byteOffset;

		gsl::span<const uint8_t, gsl::dynamic_extent> normalData
		{
			normalBufferViewStart + normalAccessor.byteOffset,
			normalBufferViewStart + normalBufferView.byteLength
		};

		std::vector<glm::vec3> normals{};
		normals.resize(static_cast<size_t>(normalAccessor.count));

		for (uint32_t normalIndex{ 0 }; normalIndex < normalAccessor.count; ++normalIndex)
		{
			uint32_t index = normalIndex * sizeof(glm::vec3);

			std::memcpy(&(normals[normalIndex]), &(normalData[index]), sizeof(glm::vec3));
		}

		const fx::gltf::Accessor& texCoordAccessor = gltf.accessors[primitve.attributes.at("TEXCOORD_0")];
		const fx::gltf::BufferView& texCoordBufferView = gltf.bufferViews[texCoordAccessor.bufferView];
		const fx::gltf::Buffer& texCoordBuffer = gltf.buffers[texCoordBufferView.buffer];
		const uint8_t* texCoordBufferViewStart = texCoordBuffer.data.data() + texCoordBufferView.byteOffset;

		gsl::span<const uint8_t, gsl::dynamic_extent> texCoordData
		{
			texCoordBufferViewStart + texCoordAccessor.byteOffset,
			texCoordBufferViewStart + texCoordBufferView.byteLength
		};

		std::vector<glm::vec2> texCoords{};
		texCoords.resize(static_cast<size_t>(texCoordAccessor.count));

		for (uint32_t texCoordIndex{ 0 }; texCoordIndex < texCoordAccessor.count; ++texCoordIndex)
		{
			uint32_t index = texCoordIndex * sizeof(glm::vec2);

			std::memcpy(&(texCoords[texCoordIndex]), &(texCoordData[index]), sizeof(glm::vec2));
		}

		meshResource.value->posistions.reserve(indices.size());
		for (uint32_t index : indices)
		{
			meshResource.value->posistions.push_back(positions[index]);
			meshResource.value->normals.push_back(normals[index]);
			glm::vec2 texCoord = settings.flipUV ? glm::vec2{ texCoords[index].x, 1.0f - texCoords[index].y } : texCoords[index];
			meshResource.value->texCoords.push_back(texCoord);
		}

		ConvertMaterial(gltf, gltf.materials[primitve.material]);
	}
}

void ResourceManager::ConvertMaterial(const fx::gltf::Document& gltf, const fx::gltf::Material& gltfMaterial)
{
	Resource<Material>& materialResource = materials.emplace_back();
	materialResource.value = std::make_unique<Material>();
	materialResource.name = gltfMaterial.name;

	if (gltfMaterial.pbrMetallicRoughness.baseColorTexture.index != -1)
	{
		const fx::gltf::Texture& gltfTexture = gltf.textures[gltfMaterial.pbrMetallicRoughness.baseColorTexture.index];

		Resource<Texture>& textureResource = textures.emplace_back();
		textureResource.value = std::make_unique<Texture>();
		textureResource.name = gltfTexture.name;

		const fx::gltf::Image& image = gltf.images[gltfTexture.source];

		const auto imagePath = fx::gltf::detail::GetDocumentRootPath(*currentAssetPath).append(image.uri);

		int width, height, channels, desiredChannels = 3;
		uint8_t* imageData = stbi_load(imagePath.string().c_str(), &width, &height, &channels, desiredChannels);
		size_t imageDataSize = static_cast<size_t>(width) * static_cast<size_t>(height)* static_cast<size_t>(desiredChannels);
		textureResource.value->data.resize(imageDataSize);
		std::memcpy(textureResource.value->data.data(), imageData, imageDataSize);
		textureResource.value->width = width;
		textureResource.value->height = height;
		textureResource.value->channels = desiredChannels;
		materialResource.value->albedoTexture = textureResource.value.get();
	}
	else
	{
		materialResource.value->albedoTexture = defaultAlbedo.value.get();
	}

	materialResource.value->albedoColor = glm::vec3(gltfMaterial.pbrMetallicRoughness.baseColorFactor[0], gltfMaterial.pbrMetallicRoughness.baseColorFactor[1], gltfMaterial.pbrMetallicRoughness.baseColorFactor[2]);
}

const Mesh& ResourceManager::GetMeshByIndex(size_t index)
{
	return *(meshes[index].value);
}

const Mesh& ResourceManager::GetMeshByName(std::string_view name)
{
	for (auto& meshResource : meshes)
	{
		if (meshResource.name == name)
		{
			return *meshResource.value;
		}
	}
}

const Material& ResourceManager::GetMaterialByIndex(size_t index)
{
	return *(materials[index].value);
}