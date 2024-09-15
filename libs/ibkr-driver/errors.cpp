#include "ibkr/internal/client.hpp"
#include <iostream>

#define NOTIFICATION_LOG(logger, code, msg) \
  DEBUG_LOG(logger) << "Notification: " << "[Code: " << code << "] " << msg;

void ibkr::internal::Client::error(int reqId, int errorCode,
                                   const std::string &errorString,
                                   const std::string &advancedOrderRejectJson) {

  if (errorCode == 2158) {
    // Securities def farm ok
    connectionState.notifySecDefServerState(true);
    NOTIFICATION_LOG(logger, errorCode, errorString);
  } else if (errorCode == 2103) {
    // data farm connection lost
    connectionState.notifyDataFarmState(false);
    WARNING_LOG(logger) << "Data farm connection lost " << errorString;
  } else if (errorCode == 2104) {
    // data farm connected ok
    connectionState.notifyDataFarmState(true);
    NOTIFICATION_LOG(logger, errorCode, errorString);
  } else if (errorCode == 2105) {
    // historical data farm connection lost
    connectionState.notifyHistoricalDataFarmState(false);
    WARNING_LOG(logger) << "Historical Data farm connection lost " << errorString;
  } else if (errorCode == 2106) {
    // historical farm connected
    connectionState.notifyHistoricalDataFarmState(true);
    NOTIFICATION_LOG(logger, errorCode, errorString);
  } else {
    ERROR_LOG(logger) << "Error: " << reqId << " With Code " << errorCode << " "
                      << errorString << " Advanced order reject"
                      << advancedOrderRejectJson << std::endl;
  }
}