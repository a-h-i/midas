#include "Contract.h"
#include "TagValue.h"

#include "ibkr/internal/client.hpp"

void ibkr::internal::Client::requestHistoricalData(const Contract &contract) {
  connectionState.clientSocket->reqHistoricalData(666, contract, "", "1 W",
                                                  "30 secs", "TRADES", 0, 1,
                                                  false, TagValueListSPtr());
}