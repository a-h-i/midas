#include "broker-interface/order.hpp"
#include "terminal-ui/ui-component.hpp"
#include <atomic>
#include <boost/signals2/connection.hpp>
#include <ftxui/component/component.hpp>
#include <ftxui/component/component_base.hpp>
#include <ftxui/dom/node.hpp>

namespace ui {
using namespace ftxui;
class ProfitAndLossWindow : public UIComponenet {
  std::atomic<double> realizedPnL{0};
  boost::signals2::scoped_connection slotConn;

public:
  ProfitAndLossWindow(midas::OrderManager &manager);
  virtual Component render() const override;
};

} // namespace ui