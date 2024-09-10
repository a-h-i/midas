#pragma once

#include "EClientSocket.h"
#include "EReader.h"
#include "EReaderOSSignal.h"
#include "EWrapper.h"

#include "logging/logging.hpp"
#include <boost/asio.hpp>
#include <memory>
#include <optional>
#include <string>

namespace ibkr::internal {
class Client : public EWrapper {

public:
  Client(const boost::asio::ip::tcp::endpoint &endpoint);
  virtual ~Client();

  virtual void nextValidId(OrderId order);
  /**
   *  In TWS (not IB Gateway) if there is an active network connection,
   * there will also immediately be callbacks to IBApi::EWrapper::error with
errorId as -1
   * and errorCode=2104,2106, errorMsg = “Market Data Server is ok” to
   *  indicate there is an active connection to the IB market data server.
Callbacks to
   * IBApi::EWrapper::error with errorId as -1 do not represent true ‘errors’
but only
   * notifications that a connection has been made successfully to the IB market
data farms.

IB Gateway by contrast will not make connections to market data farms until a
request is made by the IB client. Until this time the connection indicator in
the IB Gateway GUI will show a yellow color of ‘inactive’ rather than an
‘active’ green indication.

When initially making requests from an API application it is important that the
verifies that a response is received rather than proceeding assuming that the
network connection is ok and the subscription request (portfolio updates,
account information, etc) was made successfully.
   */
  virtual void error(int id, int errorCode, const std::string &errorString,
                     const std::string &advancedOrderRejectJson);

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
  virtual void tickPrice(TickerId tickerId, TickType field, double price,
                         const TickAttrib &attrib);
  /**
 *  EWrapper.tickSize (

tickerId: int. Request identifier used to track data.

field: int. the type of size being received (i.e. bid size)

size: int. the actual size. US stocks have a multiplier of 100.
)

Market data tick size callback. Handles all size-related ticks.

 */
  virtual void tickSize(TickerId tickerId, TickType field, Decimal size);
  /**
   *  EWrapper.tickString (

  tickerId: int. Request identifier used to track data.

  field: int. The type of the tick being received

  value: String. Variable containing message response.
  )

  Market data callback.

  Note: Every tickPrice is followed by a tickSize. There are also independent
  tickSize callbacks anytime the tickSize changes, and so there will be
  duplicate tickSize messages following a tickPrice.

   */
  virtual void tickString(TickerId tickerId, TickType tickType,
                          const std::string &value);
  virtual void tickOptionComputation(TickerId tickerId, TickType tickType,
                                     int tickAttrib, double impliedVol,
                                     double delta, double optPrice,
                                     double pvDividend, double gamma,
                                     double vega, double theta,
                                     double undPrice);

  /**
   *  EWrapper.tickGeneric (

  tickerId: int. Request identifier used to track data.

  field: int. The type of tick being received.

  value: double. Return value corresponding to value. See Available Tick Types
  for more details.
  )

  Returns generic data back to requester. Used for an array of tick types and is
  used to represent general evaluations.

   */
  virtual void tickGeneric(TickerId tickerId, TickType tickType, double value);
  /**
 * Bid EFP Computation 	Computed EFP bid price 	– 	IBApi.EWrapper.tickEFP
38 Ask EFP Computation 	Computed EFP ask price 	– 	IBApi.EWrapper.tickEFP
39 Last EFP Computation 	Computed EFP last price 	–
IBApi.EWrapper.tickEFP 	40 Open EFP Computation 	Computed EFP open price
– 	IBApi.EWrapper.tickEFP 	41 High EFP Computation 	Computed high
EFP traded price for the day 	– 	IBApi.EWrapper.tickEFP 	42 Low EFP
Computation 	Computed low EFP traded price for the day 	–
IBApi.EWrapper.tickEFP 	43 Close EFP Computation 	Computed closing EFP
price for previous day 	– 	IBApi.EWrapper.tickEFP 	44
 */
  virtual void tickEFP(TickerId tickerId, TickType tickType, double basisPoints,
                       const std::string &formattedBasisPoints,
                       double totalDividends, int holdDays,
                       const std::string &futureLastTradeDate,
                       double dividendImpact, double dividendsToLastTradeDate);
  /**
  *  EWrapper.orderStatus (

  orderId: int. The order’s client id.

  status: String. The current status of the order.

  filled: decimal. Number of filled positions.

  remaining: decimal. The remnant positions.

  avgFillPrice: double. Average filling price.

  permId: int. The order’s permId used by the TWS to identify orders.

  parentId: int. Parent’s id. Used for bracket and auto trailing stop orders.

  lastFillPrice: double. Price at which the last positions were filled.

  clientId: int. API client which submitted the order.

  whyHeld: String. this field is used to identify an order held when TWS is
  trying to locate shares for a short sell. The value used to indicate this is
  ‘locate’.

  mktCapPrice: double. If an order has been capped, this indicates the current
  capped price.
  )

  Gives the up-to-date information of an order every time it changes. Often
  there are duplicate orderStatus messages.


*Understanding Order Status Message*
***********************************
PendingSubmit 	indicates that you have transmitted the order, but have not yet
received confirmation that it has been accepted by the order destination.
PendingCancel 	indicates that you have sent a request to cancel the order but
have not yet received cancel confirmation from the order destination. At this
point, your order is not confirmed canceled. It is not guaranteed that the
cancellation will be successful. PreSubmitted 	indicates that a simulated order
type has been accepted by the IB system and that this order has yet to be
elected. The order is held in the IB system until the election criteria are met.
At that time the order is transmitted to the order destination as specified.
Submitted 	indicates that your order has been accepted by the system.
ApiCancelled 	after an order has been submitted and before it has been
acknowledged, an API client client can request its cancelation, producing this
state. Cancelled 	indicates that the balance of your order has been
confirmed canceled by the IB system. This could occur unexpectedly when IB or
the destination has rejected your order. Filled 	indicates that the order
has been completely filled. Market orders executions will not always trigger a
Filled status. Inactive 	indicates that the order was received by the
system but is no longer active because it was rejected or canceled.

                          */
  virtual void orderStatus(OrderId orderId, const std::string &status,
                           Decimal filled, Decimal remaining,
                           double avgFillPrice, int permId, int parentId,
                           double lastFillPrice, int clientId,
                           const std::string &whyHeld, double mktCapPrice);

  /**
   *
EWrapper.openOrder (

orderId: int. The order’s unique id

contract: Contract. The order’s Contract.

order: Order. The currently active Order.

orderState: OrderState. The order’s OrderState
)

Feeds in currently open orders.

   */
  virtual void openOrder(OrderId orderId, const Contract &, const Order &,
                         const OrderState &);

  /**
   *  EWrapper.openOrderEnd ()

Notifies the end of the open orders’ reception.

   */
  virtual void openOrderEnd();

  /**
   * Unknown, rest of comment from ibkr documentation
   * WinError 10038 	An operation was attempted on something that is not a
   * socket. This indicates socket connection was closed improperly. Typically
   * for Python clients. You may refer :
   * https://stackoverflow.com/questions/15210178/python-socket-programming-oserror-winerror-10038-an-operation-was-attempted-o
   */
  virtual void winError(const std::string &str, int lastError);

  /**
   *

  If there is a problem with the socket connection between TWS and the API
  client, for instance if TWS suddenly closes, this will trigger an exception in
  the EReader thread which is reading from the socket. This exception will also
  occur if an API client attempts to connect with a client ID that is already in
  use.

  The socket EOF is handled slightly differently in different API languages.
  For instance in Java, it is caught and sent to the client application to
   IBApi::EWrapper::error with errorCode 507: “Bad Message”.
   In C# it is caught and sent to IBApi::EWrapper::error with errorCode -1.
   The client application needs to handle this error message and use it to
  indicate that an exception has been thrown in the socket connection.
   Associated functions such as IBApi::EWrapper::connectionClosed
   and IBApi::EClient::IsConnected functions are not called automatically by the
  API code but need to be handled at the API client-level.
   */
  virtual void connectionClosed();

  /**
   *

  The IBApi.EClient.reqAccountUpdates function creates a
  subscription to the TWS through which account and portfolio information is
  delivered. This information is the exact same as the one displayed within the
  TWS’ Account Window. Just as with the TWS’ Account Window, unless there is a
  position change this information is updated at a fixed interval of three
  minutes.

  Unrealized and Realized P&L is sent to the API function
  IBApi.EWrapper.updateAccountValue function after a subscription request is
  made with IBApi.EClient.reqAccountUpdates. This information corresponds to the
  data in the TWS Account Window, and has a different source of information, a
  different update frequency, and different reset schedule than PnL data in the
  TWS Portfolio Window and associated API functions (below). In particular, the
  unrealized P&L information shown in the TWS Account Window which is sent to
  updatePortfolioValue will update either (1) when a trade for that particular
  instrument occurs or (2) every 3 minutes. The realized P&L data in the TWS
  Account Window is reset to 0 once per day.

  It is important to keep in mind that the P&L data shown in the Account Window
  and Portfolio Window will sometimes differ because there is a different source
  of information and a different reset schedule.
   EWrapper.updateAccountValue (

key: String. The value being updated.

value: String. up-to-date value

currency: String. The currency on which the value is expressed.

accountName: String. The account identifier.
)

Receives the subscribed account’s information. Only one account can be
subscribed at a time. After the initial callback to updateAccountValue,
callbacks only occur for values which have changed. This occurs at the time of a
position change, or every 3 minutes at most. This frequency cannot be adjusted.

   */
  virtual void updateAccountValue(const std::string &key,
                                  const std::string &val,
                                  const std::string &currency,
                                  const std::string &accountName);

  /**
   *  EWrapper.updatePortfolio (

contract: Contract. The Contract for which a position is held.

position: Decimal. The number of positions held.

marketPrice: Double. The instrument’s unitary price

marketValue: Double. Total market value of the instrument.

averageCost: Double. Average cost of the overall position.

unrealizedPNL: Double. Daily unrealized profit and loss on the position.

realizedPNL: Double. Daily realized profit and loss on the position.

accountName: String. Account ID for the update.

)

Receives the subscribed account’s portfolio. This function will receive only the
portfolio of the subscribed account. After the initial callback to
updatePortfolio, callbacks only occur for positions which have changed.

   */
  virtual void updatePortfolio(const Contract &contract, Decimal position,
                               double marketPrice, double marketValue,
                               double averageCost, double unrealizedPNL,
                               double realizedPNL,
                               const std::string &accountName);
  /**
   * Wrapper.updateAccountTime (

timestamp: String. the last update system time.

)

Receives the last time on which the account was updated.
   */
  virtual void updateAccountTime(const std::string &timeStamp);
  /**
   *  EWrapper.accountDownloadEnd (

account: String. The account identifier.

)

Notifies when all the account’s information has finished.

   */
  virtual void accountDownloadEnd(const std::string &accountName);

  /**
   * The function EClient.reqContractDetails when used with a
   *  Contract object will return contractDetails object to the contractDetails
   * function which has a list of the valid exchanges where the instrument
trades.
   * Also within the contractDetails object is a field called marketRuleIDs
which has a
   * list of “market rules”. A market rule is defined as a rule which defines
the minimum
   * price increment given the price. The market rule list returned in
contractDetails has a
   * list of market rules in the same order as the list of valid exchanges. In
this way,
   * the market rule ID for a contract on a particular exchange can be
determined.

    Market rule for forex and forex CFDs indicates default configuration
     (1/2 and not 1/10 pips). It can be adjusted to 1/10 pips through TWS or IB
Gateway Global Configuration. Some non-US securities, for instance on the SEHK
exchange, have a minimum lot size. This information is not available from the
API but can be obtained from the IB Contracts and Securities search page. It
will also be indicated in the error message returned from an order which does
not conform to the minimum lot size. Complete details about a contract in IB’s
database can be retrieved using the function IBApi.EClient.reqContractDetails.
This includes information about a contract’s conID, symbol, local symbol,
currency, etc. which is returned in a IBApi.ContractDetails object.
reqContractDetails takes as an argument a Contract object which may uniquely
match one contract, and unlike other API functions it can also take a Contract
object which matches multiple contracts in IB’s database. When there are
multiple matches, they will each be returned individually to the function
 IBApi::EWrapper::contractDetails.
   */
  virtual void contractDetails(int reqId,
                               const ContractDetails &contractDetails);
  virtual void bondContractDetails(int reqId,
                                   const ContractDetails &contractDetails) {
    WARNING_LOG(logger) << "Stub bondContractDetails unsupported instrument"
  }
  virtual void contractDetailsEnd(int reqId);
  /**
   * Order Execution callbacks
   */
  virtual void execDetails(int reqId, const Contract &contract,
                           const Execution &execution);
  /**
   * End of execution for request
   *  */
  virtual void execDetailsEnd(int reqId);

  virtual void updateMktDepth(TickerId id, int position, int operation,
                              int side, double price, Decimal size) {
    ERROR_LOG(logger) << "No support for market depth at the moment";
  }
  virtual void updateMktDepthL2(TickerId id, int position,
                                const std::string &marketMaker, int operation,
                                int side, double price, Decimal size,
                                bool isSmartDepth) {
    ERROR_LOG(logger) << "No support for market depth at the moment";
  }
  virtual void updateNewsBulletin(int msgId, int msgType,
                                  const std::string &newsMessage,
                                  const std::string &originExch) {
    ERROR_LOG(logger) << "No support for news bulletins atm";
  };
  /**
   *  EWrapper.managedAccounts (

accountsList: String. A comma-separated string with the managed account ids.

)

Returns a string of all available accounts for the logged in user. Occurs automatically on initial API client connection.

   */
  virtual void managedAccounts(const std::string &accountsList);

private:
  /**
   * Signal / mutex pthreads based
   */
  EReaderOSSignal readerSignal;
  std::unique_ptr<EClientSocket> clientSocket;
  /**
   * Order of appearance is important so that reader is destructed
   * before socket
   */
  std::unique_ptr<EReader> reader;
  boost::asio::ip::tcp::endpoint endpoint;

  std::optional<OrderId> nextValidOrderId;

  logging::thread_safe_logger_t logger;
};
} // namespace ibkr::internal
