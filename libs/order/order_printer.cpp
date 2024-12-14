#include "broker-interface/order_printer.hpp"
#include "broker-interface/order.hpp"

using namespace midas;

void OrderPrinter::visit(SimpleOrder &order) {

  ss << order.state() << " " << order.direction << " " << order.execType << " x"
     << order.requestedQuantity << " " << order.instrument << " @ "
     << order.targetPrice <<  "\n";
}

void OrderPrinter::visit(BracketedOrder &order) {
  ss << "Bracket:\n"
     << "\tEntry: ";
  visit(order.getEntryOrder());
  ss << "\tProfit Taker: ";
  visit(order.getProfitTakerOrder());
  ss << "\tStop loss: ";
  visit(order.getStopOrder());
}

std::string OrderPrinter::str() { return ss.str(); }