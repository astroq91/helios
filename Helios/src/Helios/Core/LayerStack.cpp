#include "LayerStack.h"

namespace Helios {
void LayerStack::push_layer(Layer *layer) {
  m_layers.emplace(m_layers.begin() + m_layer_insert_index, layer);
  m_layer_insert_index++;
}

void LayerStack::push_overlay(Layer *layer) { m_layers.emplace_back(layer); }

void LayerStack::pop_overlay(Layer *layer) {
  auto it =
      std::find(m_layers.begin() + m_layer_insert_index, m_layers.end(), layer);
  if (it != m_layers.end()) {
      layer->on_detach();
    m_layers.erase(it);
  }
}

void LayerStack::pop_layer(Layer *layer) {
  auto it =
      std::find(m_layers.begin(), m_layers.begin() + m_layer_insert_index, layer);
  if (it != m_layers.begin() + m_layer_insert_index) {
      layer->on_detach();
    m_layers.erase(it);
    m_layer_insert_index--;
  }
}
} // namespace Helios
