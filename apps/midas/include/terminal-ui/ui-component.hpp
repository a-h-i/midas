#pragma once

#include <ftxui/component/component_base.hpp>
#include <optional>

namespace ui {
class UIComponenet {

  std::optional<ftxui::Component> lastPainted;

protected:
  virtual bool dirty() const = 0;
  virtual ftxui::Component paint() = 0;

public:
  virtual ~UIComponenet() = default;
  ftxui::Component renderer();
};
} // namespace ui