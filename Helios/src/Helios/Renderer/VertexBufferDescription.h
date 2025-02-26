#pragma once

#include <volk/volk.h>
#include <vector>
#include <glm/glm.hpp>

namespace Helios
{
	enum class VertexAttributeFormat
	{
		FLOAT = VK_FORMAT_R32_SFLOAT,
		FLOAT2 = VK_FORMAT_R32G32_SFLOAT,
		FLOAT3 = VK_FORMAT_R32G32B32_SFLOAT,
		FLOAT4 = VK_FORMAT_R32G32B32A32_SFLOAT,
		INT32 = VK_FORMAT_R32_SINT,
		UINT32 = VK_FORMAT_R32_UINT,
	};

	struct VertexAttribute
	{
		VertexAttributeFormat format;
		uint32_t location;
	};

	enum class VertexInputRate
	{
		Vertex = VK_VERTEX_INPUT_RATE_VERTEX,
		Instance = VK_VERTEX_INPUT_RATE_INSTANCE,
	};

	inline uint32_t GetAttributeSize(const VertexAttribute& attribute)
	{
		switch (attribute.format)
		{
		case VertexAttributeFormat::FLOAT: return sizeof(float);
		case VertexAttributeFormat::FLOAT2: return 2 * sizeof(float);
		case VertexAttributeFormat::FLOAT3: return 3 * sizeof(float);
		case VertexAttributeFormat::FLOAT4: return 4 * sizeof(float);
		case VertexAttributeFormat::INT32: return sizeof(int32_t);
		case VertexAttributeFormat::UINT32: return sizeof(uint32_t);
		}

		return 0;
	}

	inline uint32_t GetAttributeAlignment(const VertexAttribute& attribute)
	{
		switch (attribute.format)
		{
		case VertexAttributeFormat::FLOAT: return 4;
		case VertexAttributeFormat::FLOAT2: return 8;
		case VertexAttributeFormat::FLOAT3: return 16;
		case VertexAttributeFormat::FLOAT4: return 16;
		case VertexAttributeFormat::INT32:
		case VertexAttributeFormat::UINT32: return 4;
		default: return 1;
		}
	}

	struct VertexBufferDescription
	{
		VertexBufferDescription(VertexInputRate inputRate, uint32_t binding,
		                        const std::vector<VertexAttribute>& attributes)
		{
			attribute_description.resize(attributes.size());

			uint32_t offset = 0;
			uint32_t max_alignment = 0;

			for (size_t i = 0; i < attributes.size(); i++)
			{
				uint32_t attribute_size = GetAttributeSize(attributes[i]);
				uint32_t alignment = GetAttributeAlignment(attributes[i]);

				offset = (offset + alignment - 1) & ~(alignment - 1);

                attribute_description[i].binding = binding;
                attribute_description[i].location = attributes[i].location;
                attribute_description[i].format = static_cast<VkFormat>(attributes[i].format);
                attribute_description[i].offset = offset;

				offset += attribute_size;
                max_alignment = std::max(max_alignment, alignment);
			}

			uint32_t stride = (offset + max_alignment - 1) & ~(max_alignment - 1);

            binding_description.binding = binding;
            binding_description.stride = stride;
            binding_description.inputRate = static_cast<VkVertexInputRate>(inputRate);
		}

		VertexBufferDescription() = default;

		VkVertexInputBindingDescription binding_description;
		std::vector<VkVertexInputAttributeDescription> attribute_description;
	};
}
