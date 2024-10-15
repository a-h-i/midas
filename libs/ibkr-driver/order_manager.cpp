#include "ibkr/internal/ibkr_order_manager.hpp"

ibkr::internal::OrderManager::OrderManager(
    std::shared_ptr<logging::thread_safe_logger_t> logger, Driver &driver)
    : midas::OrderManager(logger), driver(driver) {}







 void ibkr::internal::OrderManager::transmit(std::shared_ptr<midas::Order>) {
  
 }
 bool ibkr::internal::OrderManager::hasActiveOrders() const {
 }
 std::generator<midas::Order *> ibkr::internal::OrderManager::getFilledOrders() {}