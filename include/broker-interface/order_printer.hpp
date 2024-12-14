

#include "broker-interface/order.hpp"
#include <sstream>
namespace midas {
class OrderPrinter : public OrderVisitor {
  std::stringstream ss;
  
  public:
    std::string str();
    void visit(SimpleOrder &) override;
    void visit(BracketedOrder &) override;
};
} // namespace midas