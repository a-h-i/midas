#pragma once

#include <ftxui/component/component_base.hpp>

namespace ui {
struct UIComponenet {
  virtual ~UIComponenet() = default;
  virtual ftxui::Component render() const = 0;
};
} // namespace ui