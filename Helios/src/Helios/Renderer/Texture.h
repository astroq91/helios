#pragma once

#include <filesystem>
#include <string>
#include <volk/volk.h>

#include "DescriptorSet.h"
#include "Helios/Assets/Asset.h"
#include "Helios/Core/Core.h"
#include "Image.h"

namespace Helios {

struct CubeMapInfo {
    std::filesystem::path right;
    std::filesystem::path left;
    std::filesystem::path top;
    std::filesystem::path bottom;
    std::filesystem::path front;
    std::filesystem::path back;
};

class Texture : public Asset {
  public:
    /**
     * \brief create a texture from path.
     * \param path The path.
     * \return The texture
     */
    static SharedPtr<Texture>
    create(const std::filesystem::path& path,
           VkFormat format = VK_FORMAT_R8G8B8A8_SRGB) {
        SharedPtr<Texture> obj = SharedPtr<Texture>::create();
        obj->init_asset(path.string());
        if (!obj->init(path, format)) {
            return nullptr;
        }
        return obj;
    }

    static SharedPtr<Texture>
    create(const CubeMapInfo& cube_map_info, const std::string& name,
           VkFormat format = VK_FORMAT_R8G8B8A8_SRGB) {
        SharedPtr<Texture> obj = SharedPtr<Texture>::create();
        obj->init_asset(name);
        if (!obj->init_cube_map(cube_map_info, format)) {
            return nullptr;
        }
        return obj;
    }

    /**
     * \brief create a texture from data.
     * \param name The name of the texture.
     * \param data The raw data.
     * \param width The image width.
     * \param height The image height.
     * \param size The size (in bytes) of the image.
     * \return The texture
     */
    static SharedPtr<Texture>
    create(const std::string& name, void* data, uint32_t width, uint32_t height,
           size_t size, VkFormat format = VK_FORMAT_R8G8B8A8_SRGB) {
        SharedPtr<Texture> obj = SharedPtr<Texture>::create();
        obj->init_asset(name);
        if (!obj->init(data, width, height, size, format)) {
            return nullptr;
        }
        return obj;
    }

    const SharedPtr<Image>& get_image() const { return m_image; }

    int32_t GetTextureIndex() const { return m_texture_index; }

    bool is_cube_map() const { return m_cube_map; }
    const CubeMapInfo& get_cube_map_info() const { return m_cube_map_info; }

    Texture() = default;
    ~Texture();

    Texture(const Texture&) = delete;
    Texture& operator=(const Texture&) = delete;
    Texture(Texture&&) = delete;
    Texture& operator=(Texture&&) = delete;

  private:
    bool init(const std::filesystem::path& path, VkFormat format);
    bool init_cube_map(const CubeMapInfo& cube_map_info, VkFormat format);
    bool init(void* data, uint32_t width, uint32_t height, size_t size,
              VkFormat format);

  private:
    int32_t m_texture_index;
    bool m_cube_map = false;

    SharedPtr<Image> m_image;
    CubeMapInfo m_cube_map_info;
};
} // namespace Helios
