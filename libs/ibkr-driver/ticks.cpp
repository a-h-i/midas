#include "ibkr/internal/client.hpp"
#include <iostream>

/**
 * From IBKR documentation
 *  EWrapper.tickPrice (

tickerId: int. Request identifier used to track data.

field: int. The type of the price being received (i.e. ask price).

price: double. The actual price.

attribs: TickAttrib. A TickAttrib object that contains price attributes such as
TickAttrib::CanAutoExecute, TickAttrib::PastLimit and TickAttrib::PreOpen.
)

Market data tick price callback. Handles all price related ticks. Every
tickPrice callback is followed by a tickSize. A tickPrice value of -1 or 0
followed by a tickSize of 0 indicates there is no data for this field currently
available, whereas a tickPrice with a positive tickSize indicates an active
quote of 0 (typically for a combo contract).

 */
void ibkr::internal::Client::tickPrice(TickerId tickerId, TickType field,
                                       double price, const TickAttrib &attrib) {
  std::cout << "TickPrice: " << field << " Price: " << price
            << " attributes: " << attrib.canAutoExecute << attrib.pastLimit
            << attrib.preOpen << std::endl;
}

/**
 *  EWrapper.tickSize (

tickerId: int. Request identifier used to track data.

field: int. the type of size being received (i.e. bid size)

size: int. the actual size. US stocks have a multiplier of 100.
)

Market data tick size callback. Handles all size-related ticks.

 */
void ibkr::internal::Client::tickSize(TickerId tickerId, TickType field,
                                      Decimal size) {
  std::cout << "TickSize: " << size << " type: " << field << std::endl;
}

/**
 *  EWrapper.tickString (

tickerId: int. Request identifier used to track data.

field: int. The type of the tick being received

value: String. Variable containining message response.
)

Market data callback.

Note: Every tickPrice is followed by a tickSize. There are also independent
tickSize callbacks anytime the tickSize changes, and so there will be duplicate
tickSize messages following a tickPrice.

 */
void ibkr::internal::Client::tickString(TickerId tickerId, TickType tickType,
                                        const std::string &value) {
  std::cout << "TICK STRING: " << value << std::endl;
}