#pragma once

#include "CommonDefs.h"
#include "EClientSocket.h"
#include "EReader.h"
#include "EReaderOSSignal.h"
#include "EWrapper.h"

#include "active_subscription_state.hpp"
#include "broker-interface/subscription.hpp"
#include "exceptions/subscription_error.hpp"
#include "logging/logging.hpp"
#include <atomic>
#include <boost/asio.hpp>
#include <boost/signals2.hpp>
#include <boost/signals2/connection.hpp>
#include <deque>
#include <functional>
#include <memory>
#include <mutex>
#include <ostream>
#include <string>
#include <unordered_map>
#include <vector>

namespace ibkr::internal {

class Client : public EWrapper {

  /**
   * Before we are ready to send requests to IBKR API
   * We need more than just a socket connection.
   * We must also track data farm connections
   * as well as having a valid next order id
   */
  class ConnectivityState {
    typedef boost::signals2::signal<void(bool)> connection_signal_t;

  public:
    /**
     *  next valid order id that can be used
     */
    std::atomic<OrderId> nextValidId{-1};
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

    ConnectivityState(EWrapper *);
    ~ConnectivityState();

    void connect(const boost::asio::ip::tcp::endpoint &endpoint);
    void disconnect();

    void notifySecDefServerState(bool connected);
    void notifyDataFarmState(bool connected);
    void notifyHistoricalDataFarmState(bool connected);
    void notifyManagedAccountsReceived();

    inline bool receivedFirstValidId() const {return nextValidId.load() != -1;}

    inline bool ready() const {
      return receivedManagedAccounts && securityDefinitionServerOk &&
              receivedFirstValidId() && connectedDataFarmsCount > 0 &&
             connectedHistoricalDataFarmsCount > 0 &&
             clientSocket->isConnected();
    }
    friend std::ostream &operator<<(std::ostream &stream,
                                    const ConnectivityState &state) {
      std::string validIdState = state.receivedFirstValidId()
                                     ? std::to_string(state.nextValidId.load(std::memory_order_relaxed))
                                     : " no order id received";

      stream << "[ready: " << state.ready() << "]"
             << "[ next valid order id: " << validIdState << "]"
             << "[ count farms: "
             << state.connectedDataFarmsCount.load(std::memory_order::relaxed)
             << " ][ count historical farms: "
             << state.connectedHistoricalDataFarmsCount.load(
                    std::memory_order::relaxed)
             << " ]";
      return stream;
    }
    boost::signals2::connection
    addConnectListener(const connection_signal_t::slot_type &obs) {
      return connectionSignal.connect(obs);
    }

  private:
    bool receivedManagedAccounts = false, securityDefinitionServerOk = false;
    std::atomic<int> connectedDataFarmsCount, connectedHistoricalDataFarmsCount;
    connection_signal_t connectionSignal;
  };

public:
  Client(const boost::asio::ip::tcp::endpoint &endpoint);
  virtual ~Client();
  inline boost::signals2::connection
  addConnectListener(const std::function<void(bool)> &obs) {
    return connectionState.addConnectListener(obs);
  }
  /**
   * Does nothing if not connected
   */
  void connect();
  /**
   * Does nothing if connected
   */
  void disconnect();
  bool isConnected() const;
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
  virtual void tickOptionComputation(
      [[maybe_unused]] TickerId tickerId, [[maybe_unused]] TickType tickType,
      [[maybe_unused]] int tickAttrib, [[maybe_unused]] double impliedVol,
      [[maybe_unused]] double delta, [[maybe_unused]] double optPrice,
      [[maybe_unused]] double pvDividend, [[maybe_unused]] double gamma,
      [[maybe_unused]] double vega, [[maybe_unused]] double theta,
      [[maybe_unused]] double undPrice) {
    ERROR_LOG(logger) << "Unsupported options";
  }

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
  virtual void
  bondContractDetails([[maybe_unused]] int reqId,
                      [[maybe_unused]] const ContractDetails &contractDetails) {
    WARNING_LOG(logger) << "Stub bondContractDetails unsupported instrument";
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

  virtual void
  updateMktDepth([[maybe_unused]] TickerId id, [[maybe_unused]] int position,
                 [[maybe_unused]] int operation, [[maybe_unused]] int side,
                 [[maybe_unused]] double price, [[maybe_unused]] Decimal size) {
    ERROR_LOG(logger) << "No support for market depth at the moment";
  }
  virtual void
  updateMktDepthL2([[maybe_unused]] TickerId id, [[maybe_unused]] int position,
                   [[maybe_unused]] const std::string &marketMaker,
                   [[maybe_unused]] int operation, [[maybe_unused]] int side,
                   [[maybe_unused]] double price, [[maybe_unused]] Decimal size,
                   [[maybe_unused]] bool isSmartDepth) {
    ERROR_LOG(logger) << "No support for market depth at the moment";
  }
  virtual void
  updateNewsBulletin([[maybe_unused]] int msgId, [[maybe_unused]] int msgType,
                     [[maybe_unused]] const std::string &newsMessage,
                     [[maybe_unused]] const std::string &originExch) {
    ERROR_LOG(logger) << "No support for news bulletins atm";
  };
  /**
   *  EWrapper.managedAccounts (

accountsList: String. A comma-separated string with the managed account ids.

)

Returns a string of all available accounts for the logged in user. Occurs
automatically on initial API client connection.

   */
  virtual void managedAccounts(const std::string &accountsList);

  /**
   *

  A limitation of the function IBApi.EClient.reqAccountUpdates is that it can
  only be used with a single account at a time. To create a subscription for
  position updates from multiple accounts, the function
  IBApi.EClient.reqPositions is available.

  Note: The reqPositions function is not available in Introducing Broker or
  Financial Advisor master accounts that have very large numbers of subaccounts
  (> 50) to optimize the performance of TWS/IB Gateway. Instead the function
  reqPositionsMulti can be used to subscribe to updates from individual
  subaccounts. Also not available with IBroker accounts configured for on-demand
  account lookup.

  After initially invoking reqPositions, information about all positions in all
  associated accounts will be returned, followed by the
  IBApi::EWrapper::positionEnd callback. Thereafter, when a position has changed
  an update will be returned to the IBApi::EWrapper::position function. To
  cancel a reqPositions subscription, invoke IBApi::EClient::cancelPositions.
   EWrapper.position(

  account: String. The account holding the position.

  contract: Contract. The position’s Contract

  pos: decimal. The number of positions held. avgCost the average cost of the
  position.

  avgCost: double. The total average cost of all trades for the currently held
  position.
  )

  Provides the portfolio’s open positions. After the initial callback (only) of
  all positions, the IBApi.EWrapper.positionEnd function will be triggered.

  For futures, the exchange field will not be populated in the position callback
  as some futures trade on multiple exchanges

   */
  virtual void position(const std::string &account, const Contract &contract,
                        Decimal position, double avgCost);
  /**
   * ndicates all the positions have been transmitted.
   * Only returned after the initial callback of EWrapper.position.
   */
  virtual void positionEnd();

  virtual void positionMulti([[maybe_unused]] int reqId,
                             [[maybe_unused]] const std::string &account,
                             [[maybe_unused]] const std::string &modelCode,
                             [[maybe_unused]] const Contract &contract,
                             [[maybe_unused]] Decimal pos,
                             [[maybe_unused]] double avgCost) {
    ERROR_LOG(logger) << "Unsupported feature";
  }
  virtual void positionMultiEnd([[maybe_unused]] int reqId) {
    ERROR_LOG(logger) << "Unsupported feature";
  }

  virtual void
  accountUpdateMulti([[maybe_unused]] int reqId,
                     [[maybe_unused]] const std::string &account,
                     [[maybe_unused]] const std::string &modelCode,
                     [[maybe_unused]] const std::string &key,
                     [[maybe_unused]] const std::string &value,
                     [[maybe_unused]] const std::string &currency) {
    ERROR_LOG(logger) << "Unsupported feature";
  }
  virtual void accountUpdateMultiEnd([[maybe_unused]] int reqId) {
    ERROR_LOG(logger) << "Unsupported feature";
  }

  /**
   *  EWrapper.receiveFA (

  faDataType: int. Receive the faDataType value specified in the requestFA. See
  FA Data Types

  faXmlData: String. The xml-formatted configuration.
  )

  Receives the Financial Advisor’s configuration available in the TWS.

  financial advisors BS
   */
  virtual void receiveFA([[maybe_unused]] faDataType pFaDataType,
                         [[maybe_unused]] const std::string &cxml) {
    ERROR_LOG(logger)
        << "received receiveFA we are not even close to a financial advisor";
  }

  /**
   * Market Data Delayed
   * reqId: int. Request identifier used to track data.

bar: Bar.
The OHLC historical data Bar. The time zone of the bar is the
time zone chosen on the TWS login screen. Smallest bar size is 1 second.


The historical data will be delivered via the EWrapper.historicalData method in
the form of candlesticks. The time zone of returned bars is the time zone chosen
in TWS on the login screen.
   */
  virtual void historicalData(TickerId reqId, const Bar &bar);
  virtual void historicalDataEnd(int reqId, const std::string &startDateStr,
                                 const std::string &endDateStr);

  virtual void scannerParameters([[maybe_unused]] const std::string &xml) {
    ERROR_LOG(logger) << "Scanner functionality is currently unsupported";
  }
  virtual void
  scannerData([[maybe_unused]] int reqId, [[maybe_unused]] int rank,
              [[maybe_unused]] const ContractDetails &contractDetails,
              [[maybe_unused]] const std::string &distance,
              [[maybe_unused]] const std::string &benchmark,
              [[maybe_unused]] const std::string &projection,
              [[maybe_unused]] const std::string &legsStr) {
    ERROR_LOG(logger) << "Scanner functionality is currently unsupported";
  }
  virtual void scannerDataEnd([[maybe_unused]] int reqId) {
    ERROR_LOG(logger) << "Scanner functionality is currently unsupported";
  };

  /**
   * RealTimeData
   */

  /**
   *

  Real time and historical data functionality is combined through the
  EClient.reqRealTimeBars request. reqRealTimeBars will create an active
  subscription that will return a single bar in real time every five seconds
  that has the OHLC values over that period. reqRealTimeBars can only be used
  with a bar size of 5 seconds.

  Important: real time bars subscriptions combine the
  limitations of both, top and historical market data.
   Make sure you observe Market Data Lines and Pacing Violations for Small Bars
    (30 secs or less). For example, no more than 60 *new* requests for real
    time bars can be made in 10 minutes, and the total number of active active
  subscriptions of all types cannot exceed the maximum allowed market data lines
  for the user.


  EWrapper.realtimeBar (

  reqId: int. Request identifier used to track data.

  time: long. The bar’s start date and time (Epoch/Unix time)

  open: double. The bar’s open point

  high: double. The bar’s high point

  low: double. The bar’s low point

  close: double. The bar’s closing point

  volume: decimal. The bar’s traded volume (only returned for TRADES data)

  WAP: decimal. The bar’s Weighted Average Price rounded to minimum increment
  (only available for TRADES).

  count: int. The number of trades during the bar’s timespan (only available for
  TRADES).
  )
   */
  virtual void realtimeBar(TickerId reqId, long time, double open, double high,
                           double low, double close, Decimal volume,
                           Decimal wap, int count);
  /**
   * TWS’s current time. TWS is synchronized with the server (not local
   * computer) using NTP and this function will receive the current time in TWS.
   */
  virtual void currentTime([[maybe_unused]] long time) {
    ERROR_LOG(logger) << "Currently has no perception of time";
  }
  virtual void fundamentalData([[maybe_unused]] TickerId reqId,
                               [[maybe_unused]] const std::string &data) {
    ERROR_LOG(logger) << "currently does not handle fundamentals";
  }
  virtual void deltaNeutralValidation(
      [[maybe_unused]] int reqId,
      [[maybe_unused]] const DeltaNeutralContract &deltaNeutralContract) {
    ERROR_LOG(logger) << "unsupported feature delta neutral contracts";
  }

  /**
   * With an exchange market data subscription, such as Network A (NYSE),
   * Network B(ARCA), or Network C(NASDAQ) for US stocks, it is possible to
   * request a snapshot of the current state of the market once instead of
   * requesting a stream of updates continuously as market values change. By
   * invoking the EClient.reqMktData function passing in true for the snapshot
   * parameter, the client application will receive the currently available
   * market data once before a EWrapper.tickSnapshotEnd event is sent 11 seconds
   * later. Snapshot requests can only be made for the default tick types; no
   * generic ticks can be specified. It is important to note that a snapshot
   * request will only return available data over the 11 second span; in some
   * cases values may not be returned for all tick types.
   */
  virtual void tickSnapshotEnd(int reqId);
  /**
   *  EWrapper.marketDataType (

reqId: int. Request identifier used to track data.

marketDataType: int. Type of market data to retrieve.
)
Copy Location

1) If user sends reqMarketDataType(1) – TWS will start sending only regular (1)
market data.

2) If user sends reqMarketDataType(2) – frozen, TWS will start sending regular
(1) as default and frozen (2) market data. TWS sends marketDataType callback (1
or 2) indicating what market data will be sent after this callback. It can be
regular or frozen.

3) If user sends reqMarketDataType(3) – delayed, TWS will start sending regular
(1) as default and delayed (3) market data.

4) If user sends reqMarketDataType(4) – delayed-frozen, TWS will start sending
regular (1) as default, delayed (3) and delayed-frozen (4) market data.
   */
  virtual void marketDataType(TickerId reqId, int marketDataType);

  /**
   *

  When an order is filled either fully or partially, the
  IBApi.EWrapper.execDetails and IBApi.EWrapper.commissionReport events will
  deliver IBApi.Execution and IBApi.CommissionReport objects. This allows to
  obtain the full picture of the order’s execution and the resulting
  commissions.

      Advisors executing allocation orders will receive execution details and
  commissions for the allocation order itself. To receive allocation details and
  commissions for a specific subaccount IBApi.EClient.reqExecutions can be used.

  EWrapper.commissionReport (

  commissionReport: CommissionReport. Returns a commissions report object
  containing the fields execId, commission, currency, realizedPnl, yield, and
  yieldRedemptionDate.
  )

  Provides the CommissionReport of an Execution

   */
  virtual void commissionReport(const CommissionReport &commissionReport);
  virtual void accountSummary(int reqId, const std::string &account,
                              const std::string &tag, const std::string &value,
                              const std::string &currency);
  virtual void accountSummaryEnd(int reqId);

  virtual void verifyMessageAPI([[maybe_unused]] const std::string &apiData) {
    ERROR_LOG(logger) << " unsupported feature verify message api";
  }
  virtual void verifyCompleted([[maybe_unused]] bool isSuccessful,
                               [[maybe_unused]] const std::string &errorText) {
    ERROR_LOG(logger) << "Unsupported feature verify completed";
  }
  virtual void displayGroupList([[maybe_unused]] int reqId,
                                [[maybe_unused]] const std::string &groups) {
    ERROR_LOG(logger) << "unsupported feature display groups";
  }
  virtual void
  displayGroupUpdated([[maybe_unused]] int reqId,
                      [[maybe_unused]] const std::string &contractInfo) {
    ERROR_LOG(logger) << "unsupported feature display groups";
  }
  virtual void
  verifyAndAuthMessageAPI([[maybe_unused]] const std::string &apiData,
                          [[maybe_unused]] const std::string &xyzChallange) {
    ERROR_LOG(logger) << "unsupported feature auth message verify";
  }
  virtual void
  verifyAndAuthCompleted([[maybe_unused]] bool isSuccessful,
                         [[maybe_unused]] const std::string &errorText) {
    ERROR_LOG(logger) << "unsupported feature auth message verify";
  }

  /**
   * There is an alternative,
   *  deprecated mode of connection used in special cases in which the variable
   * AsyncEconnect is set to true, and the call to startAPI is only called from
   * the connectAck() function. All IB samples use the mode AsyncEconnect =
   * False.
   * @deprecated
   * callback initially acknowledging connection attempt connection handshake
   * not complete until nextValidID is received
   */
  virtual void connectAck() { DEBUG_LOG(logger) << "Connect ack"; }

  virtual void securityDefinitionOptionalParameter(
      int reqId, const std::string &exchange, int underlyingConId,
      const std::string &tradingClass, const std::string &multiplier,
      const std::set<std::string> &expirations,
      const std::set<double> &strikes);

  virtual void securityDefinitionOptionalParameterEnd(int reqId);

  /**
   *  This is only supported for registered professional advisors and hedge and
   * mutual funds who have configured Soft Dollar Tiers in Account Management
   */
  virtual void
  softDollarTiers([[maybe_unused]] int reqId,
                  [[maybe_unused]] const std::vector<SoftDollarTier> &tiers) {
    ERROR_LOG(logger) << "Unsupported feature";
  }

  /* It is possible to determine from the API whether an account exists under an
  account family, and find the family code using the function reqFamilyCodes.

  For instance, if individual account U112233 is under a financial advisor with
  account number F445566, if the function reqFamilyCodes is invoked for the user
  of account U112233, the family code “F445566A” will be returned, indicating
  that it belongs within that account family.
   */
  virtual void
  familyCodes([[maybe_unused]] const std::vector<FamilyCode> &familyCodes) {
    ERROR_LOG(logger) << "Unsupported feature";
  };
  virtual void
  symbolSamples([[maybe_unused]] int reqId,
                [[maybe_unused]] const std::vector<ContractDescription>
                    &contractDescriptions) {
    ERROR_LOG(logger) << "Unsupported feature";
  };
  virtual void
  mktDepthExchanges([[maybe_unused]] const std::vector<DepthMktDataDescription>
                        &depthMktDataDescriptions) {
    ERROR_LOG(logger) << "Unsupported feature";
  };
  virtual void tickNews([[maybe_unused]] int tickerId,
                        [[maybe_unused]] time_t timeStamp,
                        [[maybe_unused]] const std::string &providerCode,
                        [[maybe_unused]] const std::string &articleId,
                        [[maybe_unused]] const std::string &headline,
                        [[maybe_unused]] const std::string &extraData) {
    ERROR_LOG(logger) << "Unsupported feature";
  };
  /**
   * To find the full exchange name corresponding to a single letter code
  returned in tick types 32, 33, or 84, and API function
  IBApi::EClient::reqSmartComponents is available.
   *  Note: This function can only be used when the exchange is open
   *  EClient.reqSmartComponents (

  reqId: int. Request identifier used to track data.

  bboExchange: String. Mapping identifier received from EWrapper.tickReqParams
  )

  Returns the mapping of single letter codes to exchange names given the mapping
  identifier.

  m_pClient->reqSmartComponents(13002, m_bboExchange);


  reqId: int. Request identifier used to track data.

  smartComponentMap: SmartComponentMap. Unique object containing a map of all
  key-value pairs
  )

  Containing a bit number to exchange + exchange abbreviation dictionary. All
  IDs can be initially retrieved using reqTickParams.
   */
  virtual void
  smartComponents([[maybe_unused]] int reqId,
                  [[maybe_unused]] const SmartComponentsMap &theMap) {
    ERROR_LOG(logger) << "Unsupported feature";
  }

  /**
   * tickerId: int. Request identifier used to track data.

minTick: Minimum tick for the contract on the exchange.

bboExchange: String. Exchange offering the best bid offer.

snapshotPermissions: Based on the snapshot parameter in EClient.reqMktData.
)

Displays the ticker with BBO exchange.
   */
  virtual void tickReqParams([[maybe_unused]] int tickerId, double,
                             [[maybe_unused]] const std::string &bboExchange,
                             [[maybe_unused]] int snapshotPermissions) {
    ERROR_LOG(logger) << "Unsupported feature";
  }
  virtual void newsProviders(
      [[maybe_unused]] const std::vector<NewsProvider> &newsProviders) {
    ERROR_LOG(logger) << "Unsupported feature";
  }
  virtual void newsArticle([[maybe_unused]] int requestId,
                           [[maybe_unused]] int articleType,
                           [[maybe_unused]] const std::string &articleText) {
    ERROR_LOG(logger) << "Unsupported feature";
  }
  virtual void historicalNews([[maybe_unused]] int requestId,
                              [[maybe_unused]] const std::string &time,
                              [[maybe_unused]] const std::string &providerCode,
                              [[maybe_unused]] const std::string &articleId,
                              [[maybe_unused]] const std::string &headline) {
    ERROR_LOG(logger) << "Unsupported feature";
  }
  virtual void historicalNewsEnd([[maybe_unused]] int requestId,
                                 [[maybe_unused]] bool hasMore) {
    ERROR_LOG(logger) << "Unsupported feature";
  }

  /**
   * Given that you may not know how long a symbol has been available, you can
  use EClient.reqHeadTimestamp
   *  to find the first available point of data for a given whatToShow value
   * requestId: int. Request identifier used to track data.

  headTimestamp: String. Value identifying earliest data date
  )

  The data requested will be returned to EWrapper.headTimeStamp.
   */
  virtual void
  headTimestamp([[maybe_unused]] int reqId,
                [[maybe_unused]] const std::string &headTimestamp) {
    ERROR_LOG(logger) << "Unsupported feature";
  }
  virtual void histogramData(int reqId, const HistogramDataVector &data);
  /**
   * reqId: int. Request identifier used to track data.

bar: Bar. The OHLC historical data Bar. The time zone of the bar is the time
zone chosen on the TWS login screen. Smallest bar size is 1 second.
)

Receives bars in real time if keepUpToDate is set as True in reqHistoricalData.
Similar to realTimeBars function, except returned data is a composite of
historical data and real time data that is equivalent to TWS chart functionality
to keep charts up to date. Returned bars are successfully updated using real
time data
   */
  virtual void historicalDataUpdate([[maybe_unused]] TickerId reqId,
                                    [[maybe_unused]] const Bar &bar) {
    ERROR_LOG(logger) << "Unsupported feature";
  }

  virtual void rerouteMktDataReq([[maybe_unused]] int reqId,
                                 [[maybe_unused]] int conid,
                                 [[maybe_unused]] const std::string &exchange) {
    ERROR_LOG(logger) << "Unsupported feature";
  }
  virtual void
  rerouteMktDepthReq([[maybe_unused]] int reqId, [[maybe_unused]] int conid,
                     [[maybe_unused]] const std::string &exchange) {
    ERROR_LOG(logger) << "Unsupported feature";
  }

  /**
   * marketRuleId: int. Market Rule ID requested.

priceIncrements: PriceIncrement[]. Returns the available price increments based
on the market rule.
)

Returns minimum price increment structure for a particular market rule ID market
rule IDs for an instrument on valid exchanges can be obtained from the
contractDetails object for that contract
   */
  virtual void marketRule(
      [[maybe_unused]] int marketRuleId,
      [[maybe_unused]] const std::vector<PriceIncrement> &priceIncrements) {
    ERROR_LOG(logger) << "Unsupported feature";
  }

  /**
   * reqId: int. Request identifier for tracking data.

  dailyPnL: double. DailyPnL updates for the account in real time

  unrealizedPnL: double. Total Unrealized PnL updates for the account in real
  time

  realizedPnL: double. Total Realized PnL updates for the account in real time
   */
  virtual void pnl(int reqId, double dailyPnL, double unrealizedPnL,
                   double realizedPnL);
  /**
   * EWrapper.pnlSingle (

  reqId: int. Request identifier used for tracking.

  pos: decimal. Current size of the position

  dailyPnL: double. DailyPnL for the position

  unrealizedPnL: double. Total unrealized PnL for the position (since inception)
  updating in real time

  realizedPnL: double. Total realized PnL for the position (since inception)
  updating in real time

  value: double. Current market value of the position.
  )
   */
  virtual void pnlSingle(int reqId, Decimal pos, double dailyPnL,
                         double unrealizedPnL, double realizedPnL,
                         double value);

  /**
   *  EWrapper.historicalTicks (

  reqId: int, id of the request

  ticks: ListOfHistoricalTick, object containing a list of tick values for the
  requested timeframe.

  done: bool, return whether or not this is the end of the historical ticks
  requested.
  )

  For whatToShow=MIDPOINT

   */
  virtual void historicalTicks(int reqId,
                               const std::vector<HistoricalTick> &ticks,
                               bool done);

  /**
   * EWrapper.historicalTicksBidAsk (

  reqId: int, id of the request

  ticks: ListOfHistoricalTick, object containing a list of tick values for the
  requested timeframe.

  done: bool, return whether or not this is the end of the historical ticks
  requested.
  )

  For whatToShow=BidAs */
  virtual void historicalTicksBidAsk(
      int reqId, const std::vector<HistoricalTickBidAsk> &ticks, bool done);

  /**
   *  EWrapper.historicalTicksLast (

reqId: int, id of the request

ticks: ListOfHistoricalTick, object containing a list of tick values for the
requested timeframe.

done: bool, return whether or not this is the end of the historical ticks
requested.
)

For whatToShow=Last & AllLast
 */
  virtual void historicalTicksLast(int reqId,
                                   const std::vector<HistoricalTickLast> &ticks,
                                   bool done);

  /**
   * EWrapper.tickByTickAllLast (

reqId: int. unique identifier of the request.

tickType: int. 0: “Last” or 1: “AllLast”.

time: long. tick-by-tick real-time tick timestamp.

price: double. tick-by-tick real-time tick last price.

size: decimal. tick-by-tick real-time tick last size.

tickAttribLast: TickAttribLast. tick-by-tick real-time last tick attribs (bit 0
– past limit, bit 1 – unreported).

exchange: String. tick-by-tick real-time tick exchange.

specialConditions: String. tick-by-tick real-time tick special conditions.
Conditions under which the operation took place (Refer to Trade Conditions Page)
)

Returns “Last” or “AllLast” tick-by-tick real-time ti
   */
  virtual void tickByTickAllLast(int reqId, int tickType, time_t time,
                                 double price, Decimal size,
                                 const TickAttribLast &tickAttribLast,
                                 const std::string &exchange,
                                 const std::string &specialConditions);
  virtual void tickByTickBidAsk(int reqId, time_t time, double bidPrice,
                                double askPrice, Decimal bidSize,
                                Decimal askSize,
                                const TickAttribBidAsk &tickAttribBidAsk);
  /**
   * reqId: int. Request identifier used to track data.

  time: long. Timestamp of the tick.

  midPoint: double. Mid point value of the tick.
  )

  Returns “MidPoint” tick-by-tick real-time tick.
   */
  virtual void tickByTickMidPoint(int reqId, time_t time, double midPoint);
  /**
   *

  The IBApi.EClient.reqOpenOrders method allows to obtain all active orders
  submitted by the client application connected with the exact same client Id
  with which the order was sent to the TWS. If client 0 invokes reqOpenOrders,
  it will cause currently open orders placed from TWS manually to be ‘bound’,
  i.e. assigned an order ID so that they can be modified or cancelled by the API
  client 0.

  When an order is bound by API client 0 there will be callback to
  IBApi::EWrapper::orderBound. This indicates the mapping between API order ID
  and permID. The IBApi.EWrapper.orderBound callback in response to newly bound
  orders that indicates the mapping between the permID (unique account-wide) and
  API Order ID (specific to an API client). In the API settings in Global
  Configuration, is a setting checked by default “Use negative numbers to bind
  automatic orders” which will specify how manual TWS orders are assigned an API
  order ID. orderId: long. IBKR permId.

  apiClientId: int. API client id.

  apiOrderId: int. API order id.

   */
  virtual void orderBound([[maybe_unused]] long long orderId,
                          [[maybe_unused]] int apiClientId,
                          [[maybe_unused]] int apiOrderId) {
    ERROR_LOG(logger) << "Unsupported feature";
  };
  /**
   * EClient.reqCompletedOrders allows users to request all
   *  orders for the given day that are no longer modifiable.
   * This will include orders have that executed, been rejected,
   *  or have been cancelled by the user. Clients may use these requests in
   * order to retain a roster of those order submissions that are no longer
   * traceable via reqOpenOrders.
   */
  virtual void completedOrder([[maybe_unused]] const Contract &contract,
                              [[maybe_unused]] const Order &order,
                              [[maybe_unused]] const OrderState &orderState) {
    ERROR_LOG(logger) << "Unsupported feature";
  }
  /** Notifies the end of the completed orders’ reception */
  virtual void completedOrdersEnd();
  virtual void replaceFAEnd([[maybe_unused]] int reqId,
                            [[maybe_unused]] const std::string &text) {
    ERROR_LOG(logger) << "Unsupported financial advisor feature";
  };
  virtual void wshMetaData([[maybe_unused]] int reqId,
                           [[maybe_unused]] const std::string &dataJson) {
    ERROR_LOG(logger) << "Wall street horizon unsupported ";
  }
  virtual void wshEventData([[maybe_unused]] int reqId,
                            [[maybe_unused]] const std::string &dataJson) {
    ERROR_LOG(logger) << "Wall street horizon unsupported ";
  }
  /**
   *  EWrapper.historicalSchedule (

  reqId: int. Request identifier used to track data.

  startDateTime: String. Returns the start date and time of the historical
  schedule range.

  endDateTime: String. Returns the end date and time of the historical schedule
  range.

  timeZone: String. Returns the time zone referenced by the schedule.

  sessions: HistoricalSession[]. Returns the full block of historical schedule
  data for the duration.
  )

  In the case of whatToShow=”schedule”, you will need to also define the
  EWrapper.historicalSchedule value. This is a unique method that will only be
  called in the case of the unique whatToShow value to display calendar
  information.

   */
  virtual void historicalSchedule(
      [[maybe_unused]] int reqId,
      [[maybe_unused]] const std::string &startDateTime,
      [[maybe_unused]] const std::string &endDateTime,
      [[maybe_unused]] const std::string &timeZone,
      [[maybe_unused]] const std::vector<HistoricalSession> &sessions) {
    ERROR_LOG(logger) << "Unsupported feature historical schedule";
  }
  /**
   * Receiving White Branding Info
  Copy Location
  EWrapper.userInfo (

  reqId: int. Identifier for the given request.

  whiteBrandingId: String. Identifier for the white branded entity.
  )

   */
  virtual void userInfo([[maybe_unused]] int reqId,
                        [[maybe_unused]] const std::string &whiteBrandingId) {
    ERROR_LOG(logger) << "unsupported feature";
  }
  /**
   * Needs to be called in a process loop
   */
  bool processCycle();
  void addSubscription(std::weak_ptr<midas::Subscription> subscription);

private:
  ConnectivityState connectionState;
  boost::asio::ip::tcp::endpoint endpoint;

  logging::thread_safe_logger_t logger;
  std::atomic<TickerId> nextTickerId;

  std::vector<std::string> managedAccountIds;
  std::recursive_mutex subscriptionsMutex;
  /**
   * Subscriptions that have not yet been processed
   */
  std::deque<std::weak_ptr<midas::Subscription>> pendingSubscriptions;
  std::unordered_map<TickerId, ActiveSubscriptionState> activeSubscriptions;

  void processPendingSubscriptions();
  /**
   * @param func processes function, subscriptionsMutex is locked during
   * invocation. If it returns true subscription is removed.
   * @param ticker ticker id to process
   * @returns number of processed subscriptions
   */
  std::size_t
  applyToActiveSubscriptions(std::function<bool(midas::Subscription &, ActiveSubscriptionState &state)> func,
                             const TickerId ticker);
  void removeActiveSubscription(const TickerId ticker);

  void handleSubscriptionCancel(const TickerId, const midas::Subscription &);
};

} // namespace ibkr::internal
