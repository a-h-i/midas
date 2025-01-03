#include "terminal-ui/ui-component.hpp"
#include <ftxui/component/component_base.hpp>

ftxui::Component ui::UIComponenet::renderer() {
  if (!lastPainted.has_value() || dirty()) {

    lastPainted = paint();
  }
  return lastPainted.value();
}