#include "ibkr/internal/client.hpp"
#include <iostream>

void ibkr::internal::Client::error(int id, int errorCode,
                                   const std::string &errorString,
                                   const std::string &advancedOrderRejectJson) {
  std::cerr << "Error: " << id << " With Code " << errorCode << "\n"
            << errorString << "\nAdvanced order reject"
            << advancedOrderRejectJson << std::endl;
}