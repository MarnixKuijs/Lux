#include "ResourceManager.h"

#include <glm/vec3.hpp>

#include <fx/gltf.h>
#include <gsl/span>
#include <gsl/multi_span>
#include <cstring>

void ResourceManager::ImportFromGltf(std::filesystem::path&& filePath)
{
	const fx::gltf::Document gltf = fx::gltf::LoadFromText(filePath.string());

	const fx::gltf::Scene& scene = gltf.scenes[gltf.scene];
	
	for (const uint32_t nodeIndex : scene.nodes)
	{		
		ParseNode(gltf, gltf.nodes[nodeIndex]);
	}
}

void ResourceManager::ParseNode(const fx::gltf::Document& gltf, const fx::gltf::Node& node)
{
	if (node.mesh != -1)
	{
		ConvertMesh(gltf, gltf.meshes[node.mesh]);
	}

	for (const uint32_t nodeIndex : node.children)
	{
		ParseNode(gltf, gltf.nodes[nodeIndex]);
	}
}

void ResourceManager::ConvertMesh(const fx::gltf::Document& gltf, const fx::gltf::Mesh gltfMesh)
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

			std::memcpy(&(positions[normalIndex]), &(positionData[index]), sizeof(glm::vec3));
		}

		meshResource.value->posistions.reserve(indices.size());
		for (uint32_t index : indices)
		{
			meshResource.value->posistions.push_back(positions[index]);
			meshResource.value->normals.push_back(normals[index]);
		}
	}
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