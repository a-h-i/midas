//
// Created by ahi on 1/11/25.
//

#include "broker-interface/order.hpp"
void midas::OrderManager::handleRealized(InstrumentEnum instrument,
                                         int quantity, double pricePerUnit) {
  positionTracker.handle_position_update(instrument, quantity, pricePerUnit);
}
