#pragma once
#include <vector>
#include <vulkan/vulkan.h>

#include "Buffer.h"
#include "DescriptorPool.h"
#include "DescriptorSetLayout.h"

namespace Helios {
enum class DescriptorClass {
  Buffer,
  Image,
};

struct DescriptorSpec {
  uint32_t binding;
  VkDescriptorType type;
  DescriptorClass descriptor_class;
  Ref<Buffer> buffer;
  VkImageView image_view;
  VkSampler sampler;
  uint32_t descriptor_count = 1;
  uint32_t dst_array_element = 0;
};

class DescriptorSet {
public:
  static Unique<DescriptorSet>
  create_unique(const Ref<DescriptorPool> &pool,
                const Ref<DescriptorSetLayout> &set_layouts,
                const std::vector<DescriptorSpec> &descriptor_specs = {}) {
    Unique<DescriptorSet> ds = make_unique<DescriptorSet>();
      ds->init(pool, set_layouts, descriptor_specs);
    return ds;
  }

  /**
   * \brief Update the set with new descriptors.
   * \param descriptor_specs The new descriptor specifications.
   */
  void update_descriptor_set(const std::vector<DescriptorSpec> &descriptor_specs);

  const VkDescriptorSet &get_vk_set() const { return m_set; }

  DescriptorSet() = default;
  ~DescriptorSet() = default;

  DescriptorSet(const DescriptorSet &) = delete;
  DescriptorSet &operator=(const DescriptorSet &) = delete;
  DescriptorSet(DescriptorSet &&) = delete;
  DescriptorSet &operator=(DescriptorSet &&) = delete;

private:
  void init(const Ref<DescriptorPool> &pool,
            const Ref<DescriptorSetLayout> &set_layout,
            const std::vector<DescriptorSpec> &descriptor_specs);

private:
  VkDescriptorSet m_set;
};
} // namespace Helios
