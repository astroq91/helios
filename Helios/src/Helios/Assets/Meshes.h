#pragma once
#include "Helios/Renderer/VertexBufferDescription.h"
#include <array>
namespace Helios {
constexpr std::array DEFAULT_MESH_ATTRIBUTES{
    VertexAttribute{VertexAttributeFormat::FLOAT3, 0}, // position
    VertexAttribute{VertexAttributeFormat::FLOAT3, 1}, // normal
    VertexAttribute{VertexAttributeFormat::FLOAT2, 2}, // Texture Coords
};
}; // namespace Helios
