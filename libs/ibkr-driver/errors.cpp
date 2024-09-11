#include "ibkr/internal/client.hpp"
#include <iostream>

void ibkr::internal::Client::error(int id, int errorCode,
                                   const std::string &errorString,
                                   const std::string &advancedOrderRejectJson) {
  ERROR_LOG(logger) << "Error: " << id << " With Code " << errorCode << " "
            << errorString << " Advanced order reject"
            << advancedOrderRejectJson << std::endl;


  
}