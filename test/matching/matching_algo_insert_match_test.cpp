#include "test_helper.h"

#include <boost/test/unit_test.hpp>

#include "matching/matching_algo.hpp"
#include "events/client_order_request.h"
#include "matching/validators/matching_validators.hpp"

using namespace codetest::matching_engine_sim;
using namespace codetest::matching_engine_sim_test_helper;

BOOST_AUTO_TEST_SUITE(PriceTimePriorityMatching_InsertAndMatch_TestSuite)

namespace codetest::matching_engine_sim_test {

BOOST_AUTO_TEST_CASE(New_LimitBuyOrder_OnEmptyOrderBook)
{
  PriceTimePriorityMatching<> matching_engine;

  ClientOrderRequest<> test_order_request{
      OrderSide::BUY,
      OrderAction::NEW,
      OrderType::LIMIT,
      GenTestOrderID(),
      DEFAULT_TEST_ORDER_SIZE,
      DEFAULT_TEST_ORDER_PRICE,
      DEFAULT_TEST_CLIENT_1_ID,
      DEFAULT_TEST_INSTRUMENT_1_ID
  };

  const ClientOrderRequest<> test_order_request_clone = test_order_request;

  PassiveOrderBook<> test_passive_order_book{};
  EngineEventTestObserver test_observer;

  matching_engine.doProcessOrderRequest(test_order_request, test_passive_order_book, test_observer);

  // Expect the buy order placed on bid queue, and empty ask queue
  const auto &ask_order_queue{test_passive_order_book.getAskOrderQueue()};
  BOOST_CHECK_EQUAL(ask_order_queue.size(), 0);

  const auto &bid_order_queue{test_passive_order_book.getBidOrderQueue()};
  BOOST_CHECK_EQUAL(bid_order_queue.size(), 1);

  auto first_bid_level_itr = bid_order_queue.begin();
  BOOST_CHECK(first_bid_level_itr != bid_order_queue.end());

  BOOST_CHECK_EQUAL(first_bid_level_itr->first, test_order_request.price_);

  const auto &persisted_passive_order = first_bid_level_itr->second.front();
  BOOST_CHECK_EQUAL(persisted_passive_order->cln_order_id_, test_order_request_clone.cln_order_id_);
  BOOST_CHECK_EQUAL(persisted_passive_order->client_, test_order_request_clone.client_);
  BOOST_CHECK_EQUAL(persisted_passive_order->remaining_size_, test_order_request_clone.size_);

  // Expect no trades reported
  BOOST_CHECK(test_observer.client_trade_events_.size() == 0);
}

BOOST_AUTO_TEST_CASE(New_LimitSellOrder_OnEmptyOrderBook)
{
  PriceTimePriorityMatching<> matching_engine;

  ClientOrderRequest<> test_order_request{
      OrderSide::SELL,
      OrderAction::NEW,
      OrderType::LIMIT,
      GenTestOrderID(),
      DEFAULT_TEST_ORDER_SIZE,
      DEFAULT_TEST_ORDER_PRICE,
      DEFAULT_TEST_CLIENT_1_ID,
      DEFAULT_TEST_INSTRUMENT_1_ID
  };

  const ClientOrderRequest<> test_order_request_clone = test_order_request;

  PassiveOrderBook<> test_passive_order_book{};
  EngineEventTestObserver test_observer;

  matching_engine.doProcessOrderRequest(test_order_request, test_passive_order_book, test_observer);

  // Expect the sell order placed on ask queue, and empty bid queue
  const auto &bid_order_queue{test_passive_order_book.getBidOrderQueue()};
  BOOST_CHECK_EQUAL(bid_order_queue.size(), 0);

  const auto &ask_order_queue{test_passive_order_book.getAskOrderQueue()};
  BOOST_CHECK_EQUAL(ask_order_queue.size(), 1);

  auto first_ask_level_itr = ask_order_queue.begin();
  BOOST_CHECK(first_ask_level_itr != ask_order_queue.end());

  BOOST_CHECK_EQUAL(first_ask_level_itr->first, test_order_request.price_);

  const auto &persisted_passive_order = first_ask_level_itr->second.front();
  BOOST_CHECK_EQUAL(persisted_passive_order->cln_order_id_, test_order_request_clone.cln_order_id_);
  BOOST_CHECK_EQUAL(persisted_passive_order->client_, test_order_request_clone.client_);
  BOOST_CHECK_EQUAL(persisted_passive_order->remaining_size_, test_order_request_clone.size_);

  // Expect no trades reported
  BOOST_CHECK(test_observer.client_trade_events_.size() == 0);
}

BOOST_AUTO_TEST_CASE(New_MarketBuyOrder_OnEmptyOrderBook)
{
  PriceTimePriorityMatching<> matching_engine;

  ClientOrderRequest<> test_order_request{
      OrderSide::BUY,
      OrderAction::NEW,
      OrderType::MARKET,
      GenTestOrderID(),
      DEFAULT_TEST_ORDER_SIZE,
      DEFAULT_TEST_ORDER_PRICE,
      DEFAULT_TEST_CLIENT_1_ID,
      DEFAULT_TEST_INSTRUMENT_1_ID
  };

  PassiveOrderBook<> test_passive_order_book{};
  EngineEventTestObserver test_observer;

  matching_engine.doProcessOrderRequest(test_order_request, test_passive_order_book, test_observer);

  // Expect no trade and no order persisted
  const auto &ask_order_queue{test_passive_order_book.getAskOrderQueue()};
  BOOST_CHECK_EQUAL(ask_order_queue.size(), 0);

  const auto &bid_order_queue{test_passive_order_book.getBidOrderQueue()};
  BOOST_CHECK_EQUAL(bid_order_queue.size(), 0);

  BOOST_CHECK(test_observer.client_trade_events_.size() == 0);
}

BOOST_AUTO_TEST_CASE(New_MarketSellOrder_OnEmptyOrderBook)
{
  PriceTimePriorityMatching<> matching_engine;

  ClientOrderRequest<> test_order_request{
      OrderSide::SELL,
      OrderAction::NEW,
      OrderType::MARKET,
      GenTestOrderID(),
      DEFAULT_TEST_ORDER_SIZE,
      DEFAULT_TEST_ORDER_PRICE,
      DEFAULT_TEST_CLIENT_1_ID,
      DEFAULT_TEST_INSTRUMENT_1_ID
  };

  PassiveOrderBook<> test_passive_order_book{};
  EngineEventTestObserver test_observer;

  matching_engine.doProcessOrderRequest(test_order_request, test_passive_order_book, test_observer);

  // Expect no trade and no order persisted
  const auto &ask_order_queue{test_passive_order_book.getAskOrderQueue()};
  BOOST_CHECK_EQUAL(ask_order_queue.size(), 0);

  const auto &bid_order_queue{test_passive_order_book.getBidOrderQueue()};
  BOOST_CHECK_EQUAL(bid_order_queue.size(), 0);

  BOOST_CHECK(test_observer.client_trade_events_.size() == 0);
}

BOOST_AUTO_TEST_CASE(New_LimitBuySell_PricesDontCross) {
  PriceTimePriorityMatching<> matching_engine;

  ClientOrderRequest<> test_order_request_buy1{
      OrderSide::BUY,
      OrderAction::NEW,
      OrderType::LIMIT,
      GenTestOrderID(),
      DEFAULT_TEST_ORDER_SIZE,
      DEFAULT_TEST_ORDER_PRICE,
      DEFAULT_TEST_CLIENT_1_ID,
      DEFAULT_TEST_INSTRUMENT_1_ID
  };

  ClientOrderRequest<> test_order_request_buy2{
      OrderSide::BUY,
      OrderAction::NEW,
      OrderType::LIMIT,
      GenTestOrderID(),
      DEFAULT_TEST_ORDER_SIZE,
      DEFAULT_TEST_ORDER_PRICE,
      DEFAULT_TEST_CLIENT_1_ID,
      DEFAULT_TEST_INSTRUMENT_1_ID
  };

  ClientOrderRequest<> test_order_request_sell1{
      OrderSide::SELL,
      OrderAction::NEW,
      OrderType::LIMIT,
      GenTestOrderID(),
      DEFAULT_TEST_ORDER_SIZE,
      DEFAULT_TEST_ORDER_PRICE + 1,
      DEFAULT_TEST_CLIENT_1_ID,
      DEFAULT_TEST_INSTRUMENT_1_ID
  };

  ClientOrderRequest<> test_order_request_sell2{
      OrderSide::SELL,
      OrderAction::NEW,
      OrderType::LIMIT,
      GenTestOrderID(),
      DEFAULT_TEST_ORDER_SIZE,
      DEFAULT_TEST_ORDER_PRICE + 1,
      DEFAULT_TEST_CLIENT_1_ID,
      DEFAULT_TEST_INSTRUMENT_1_ID
  };

  const ClientOrderRequest<> test_order_request_buy1_clone = test_order_request_buy1;
  const ClientOrderRequest<> test_order_request_buy2_clone = test_order_request_buy2;
  const ClientOrderRequest<> test_order_request_sell1_clone = test_order_request_sell1;
  const ClientOrderRequest<> test_order_request_sell2_clone = test_order_request_sell2;

  PassiveOrderBook<> test_passive_order_book{};
  EngineEventTestObserver test_observer;

  matching_engine.doProcessOrderRequest(test_order_request_buy1, test_passive_order_book, test_observer);
  matching_engine.doProcessOrderRequest(test_order_request_buy2, test_passive_order_book, test_observer);
  matching_engine.doProcessOrderRequest(test_order_request_sell1, test_passive_order_book, test_observer);
  matching_engine.doProcessOrderRequest(test_order_request_sell2, test_passive_order_book, test_observer);

  // Expect both bid ask queue for each price queued with 2 orders, no trades
  const auto &ask_order_queue{test_passive_order_book.getAskOrderQueue()};
  BOOST_CHECK_EQUAL(ask_order_queue.size(), 1);

  const auto &[best_ask_price, best_ask_order_queue] {*ask_order_queue.begin()};
  BOOST_CHECK_EQUAL(best_ask_price, test_order_request_sell1_clone.price_);
  BOOST_CHECK_EQUAL(best_ask_order_queue.size(), 2);

  const auto &bid_order_queue{test_passive_order_book.getBidOrderQueue()};
  BOOST_CHECK_EQUAL(bid_order_queue.size(), 1);

  const auto &[best_bid_price, best_bid_order_queue] {*bid_order_queue.begin()};
  BOOST_CHECK_EQUAL(best_bid_price, test_order_request_buy1_clone.price_);
  BOOST_CHECK_EQUAL(best_bid_order_queue.size(), 2);

  BOOST_CHECK(test_observer.client_trade_events_.size() == 0);
}

BOOST_AUTO_TEST_CASE(New_LimitBuyDifferentPrices_PricesDontCross) {
  PriceTimePriorityMatching<> matching_engine;

  ClientOrderRequest<> test_order_request_buy1{
      OrderSide::BUY,
      OrderAction::NEW,
      OrderType::LIMIT,
      GenTestOrderID(),
      DEFAULT_TEST_ORDER_SIZE,
      DEFAULT_TEST_ORDER_PRICE,
      DEFAULT_TEST_CLIENT_1_ID,
      DEFAULT_TEST_INSTRUMENT_1_ID
  };

  ClientOrderRequest<> test_order_request_buy2{
      OrderSide::BUY,
      OrderAction::NEW,
      OrderType::LIMIT,
      GenTestOrderID(),
      DEFAULT_TEST_ORDER_SIZE,
      DEFAULT_TEST_ORDER_PRICE - 1,
      DEFAULT_TEST_CLIENT_1_ID,
      DEFAULT_TEST_INSTRUMENT_1_ID
  };

  ClientOrderRequest<> test_order_request_sell1{
      OrderSide::SELL,
      OrderAction::NEW,
      OrderType::LIMIT,
      GenTestOrderID(),
      DEFAULT_TEST_ORDER_SIZE,
      DEFAULT_TEST_ORDER_PRICE + 1,
      DEFAULT_TEST_CLIENT_1_ID,
      DEFAULT_TEST_INSTRUMENT_1_ID
  };

  ClientOrderRequest<> test_order_request_sell2{
      OrderSide::SELL,
      OrderAction::NEW,
      OrderType::LIMIT,
      GenTestOrderID(),
      DEFAULT_TEST_ORDER_SIZE,
      DEFAULT_TEST_ORDER_PRICE + 2,
      DEFAULT_TEST_CLIENT_1_ID,
      DEFAULT_TEST_INSTRUMENT_1_ID
  };

  const ClientOrderRequest<> test_order_request_buy1_clone = test_order_request_buy1;
  const ClientOrderRequest<> test_order_request_buy2_clone = test_order_request_buy2;
  const ClientOrderRequest<> test_order_request_sell1_clone = test_order_request_sell1;
  const ClientOrderRequest<> test_order_request_sell2_clone = test_order_request_sell2;

  PassiveOrderBook<> test_passive_order_book{};
  EngineEventTestObserver test_observer;

  matching_engine.doProcessOrderRequest(test_order_request_buy1, test_passive_order_book, test_observer);
  matching_engine.doProcessOrderRequest(test_order_request_buy2, test_passive_order_book, test_observer);
  matching_engine.doProcessOrderRequest(test_order_request_sell1, test_passive_order_book, test_observer);
  matching_engine.doProcessOrderRequest(test_order_request_sell2, test_passive_order_book, test_observer);

  // Expect bid ask each 2 prices each queued with 1 orders, no trades
  const auto &ask_order_queue{test_passive_order_book.getAskOrderQueue()};
  BOOST_CHECK_EQUAL(ask_order_queue.size(), 2);

  const auto &[best_ask_price, best_ask_order_queue] {*ask_order_queue.begin()};
  BOOST_CHECK_EQUAL(best_ask_price, test_order_request_sell1_clone.price_);
  BOOST_CHECK_EQUAL(best_ask_order_queue.size(), 1);

  const auto &[next_ask_price, next_ask_order_queue] {*(++ask_order_queue.begin())};
  BOOST_CHECK_EQUAL(next_ask_price, test_order_request_sell2_clone.price_);
  BOOST_CHECK_EQUAL(next_ask_order_queue.size(), 1);

  const auto &bid_order_queue{test_passive_order_book.getBidOrderQueue()};
  BOOST_CHECK_EQUAL(bid_order_queue.size(), 2);

  const auto &[best_bid_price, best_bid_order_queue] {*bid_order_queue.begin()};
  BOOST_CHECK_EQUAL(best_bid_price, test_order_request_buy1_clone.price_);
  BOOST_CHECK_EQUAL(best_bid_order_queue.size(), 1);

  const auto &[next_bid_price, next_bid_order_queue] {*(++bid_order_queue.begin())};
  BOOST_CHECK_EQUAL(next_bid_price, test_order_request_buy2_clone.price_);
  BOOST_CHECK_EQUAL(next_bid_order_queue.size(), 1);

  BOOST_CHECK(test_observer.client_trade_events_.size() == 0);
}

BOOST_AUTO_TEST_CASE(LimitSellCrossLimitBuy_SamePrice_SingleTradeFullyFilled) {
  PriceTimePriorityMatching<> matching_engine;

  ClientOrderRequest<> test_order_request_buy{
      OrderSide::BUY,
      OrderAction::NEW,
      OrderType::LIMIT,
      GenTestOrderID(),
      DEFAULT_TEST_ORDER_SIZE,
      DEFAULT_TEST_ORDER_PRICE,
      DEFAULT_TEST_CLIENT_1_ID,
      DEFAULT_TEST_INSTRUMENT_1_ID
  };

  ClientOrderRequest<> test_order_request_sell{
      OrderSide::SELL,
      OrderAction::NEW,
      OrderType::LIMIT,
      GenTestOrderID(),
      DEFAULT_TEST_ORDER_SIZE,
      DEFAULT_TEST_ORDER_PRICE,
      DEFAULT_TEST_CLIENT_1_ID,
      DEFAULT_TEST_INSTRUMENT_1_ID
  };

  const ClientOrderRequest<> test_order_request_buy_clone = test_order_request_buy;
  const ClientOrderRequest<> test_order_request_sell_clone = test_order_request_sell;

  PassiveOrderBook<> test_passive_order_book{};
  EngineEventTestObserver test_observer;

  matching_engine.doProcessOrderRequest(test_order_request_buy, test_passive_order_book, test_observer);
  matching_engine.doProcessOrderRequest(test_order_request_sell, test_passive_order_book, test_observer);

  // Expect no order queued, 1 trade
  const auto &ask_order_queue{test_passive_order_book.getAskOrderQueue()};
  BOOST_CHECK_EQUAL(ask_order_queue.size(), 0);

  const auto &bid_order_queue{test_passive_order_book.getBidOrderQueue()};
  BOOST_CHECK_EQUAL(bid_order_queue.size(), 0);

  BOOST_CHECK(test_observer.client_trade_events_.size() == 1);
  const auto &client_trade = test_observer.client_trade_events_[0];
  BOOST_CHECK_EQUAL(client_trade.trade_price_, test_order_request_sell_clone.price_);
  BOOST_CHECK_EQUAL(client_trade.size_, test_order_request_sell_clone.size_);
  BOOST_CHECK_EQUAL(client_trade.client1_, test_order_request_sell_clone.client_);
  BOOST_CHECK_EQUAL(client_trade.client1_order_id_, test_order_request_sell_clone.cln_order_id_);
  BOOST_CHECK_EQUAL(client_trade.client2_, test_order_request_buy_clone.client_);
  BOOST_CHECK_EQUAL(client_trade.client2_order_id_, test_order_request_buy_clone.cln_order_id_);
}

BOOST_AUTO_TEST_CASE(LimitBuyCrossLimitSell_SamePrice_SingleTradeFullyFilled) {
  PriceTimePriorityMatching<> matching_engine;

  ClientOrderRequest<> test_order_request_buy{
      OrderSide::BUY,
      OrderAction::NEW,
      OrderType::LIMIT,
      GenTestOrderID(),
      DEFAULT_TEST_ORDER_SIZE,
      DEFAULT_TEST_ORDER_PRICE,
      DEFAULT_TEST_CLIENT_1_ID,
      DEFAULT_TEST_INSTRUMENT_1_ID
  };

  ClientOrderRequest<> test_order_request_sell{
      OrderSide::SELL,
      OrderAction::NEW,
      OrderType::LIMIT,
      GenTestOrderID(),
      DEFAULT_TEST_ORDER_SIZE,
      DEFAULT_TEST_ORDER_PRICE,
      DEFAULT_TEST_CLIENT_1_ID,
      DEFAULT_TEST_INSTRUMENT_1_ID
  };

  const ClientOrderRequest<> test_order_request_buy_clone = test_order_request_buy;
  const ClientOrderRequest<> test_order_request_sell_clone = test_order_request_sell;

  PassiveOrderBook<> test_passive_order_book{};
  EngineEventTestObserver test_observer;

  matching_engine.doProcessOrderRequest(test_order_request_sell, test_passive_order_book, test_observer);
  matching_engine.doProcessOrderRequest(test_order_request_buy, test_passive_order_book, test_observer);

  // Expect no order queued, 1 trade
  const auto &ask_order_queue{test_passive_order_book.getAskOrderQueue()};
  BOOST_CHECK_EQUAL(ask_order_queue.size(), 0);

  const auto &bid_order_queue{test_passive_order_book.getBidOrderQueue()};
  BOOST_CHECK_EQUAL(bid_order_queue.size(), 0);

  BOOST_CHECK(test_observer.client_trade_events_.size() == 1);
  const auto &client_trade = test_observer.client_trade_events_[0];

  BOOST_CHECK_EQUAL(client_trade.trade_price_, test_order_request_buy_clone.price_);
  BOOST_CHECK_EQUAL(client_trade.size_, test_order_request_buy_clone.size_);
  BOOST_CHECK_EQUAL(client_trade.client1_, test_order_request_buy_clone.client_);
  BOOST_CHECK_EQUAL(client_trade.client1_order_id_, test_order_request_buy_clone.cln_order_id_);
  BOOST_CHECK_EQUAL(client_trade.client2_, test_order_request_sell_clone.client_);
  BOOST_CHECK_EQUAL(client_trade.client2_order_id_, test_order_request_sell_clone.cln_order_id_);
}

BOOST_AUTO_TEST_CASE(LimitSellCrossLimitBuy_AggressivePrice_SingleTradeFullyFilled) {
  PriceTimePriorityMatching<> matching_engine;

  ClientOrderRequest<> test_order_request_buy{
      OrderSide::BUY,
      OrderAction::NEW,
      OrderType::LIMIT,
      GenTestOrderID(),
      DEFAULT_TEST_ORDER_SIZE,
      DEFAULT_TEST_ORDER_PRICE,
      DEFAULT_TEST_CLIENT_1_ID,
      DEFAULT_TEST_INSTRUMENT_1_ID
  };

  ClientOrderRequest<> test_order_request_sell{
      OrderSide::SELL,
      OrderAction::NEW,
      OrderType::LIMIT,
      GenTestOrderID(),
      DEFAULT_TEST_ORDER_SIZE,
      DEFAULT_TEST_ORDER_PRICE - 1,
      DEFAULT_TEST_CLIENT_1_ID,
      DEFAULT_TEST_INSTRUMENT_1_ID
  };

  const ClientOrderRequest<> test_order_request_buy_clone = test_order_request_buy;
  const ClientOrderRequest<> test_order_request_sell_clone = test_order_request_sell;

  PassiveOrderBook<> test_passive_order_book{};
  EngineEventTestObserver test_observer;

  matching_engine.doProcessOrderRequest(test_order_request_buy, test_passive_order_book, test_observer);
  matching_engine.doProcessOrderRequest(test_order_request_sell, test_passive_order_book, test_observer);

  const PriceType TRADE_PRICE = test_order_request_buy_clone.price_;

  // Expect no order queued, 1 trade
  const auto &ask_order_queue{test_passive_order_book.getAskOrderQueue()};
  BOOST_CHECK_EQUAL(ask_order_queue.size(), 0);

  const auto &bid_order_queue{test_passive_order_book.getBidOrderQueue()};
  BOOST_CHECK_EQUAL(bid_order_queue.size(), 0);

  BOOST_CHECK(test_observer.client_trade_events_.size() == 1);
  const auto &client_trade = test_observer.client_trade_events_[0];
  BOOST_CHECK_EQUAL(client_trade.trade_price_, TRADE_PRICE);
  BOOST_CHECK_EQUAL(client_trade.size_, test_order_request_sell_clone.size_);
  BOOST_CHECK_EQUAL(client_trade.client1_, test_order_request_sell_clone.client_);
  BOOST_CHECK_EQUAL(client_trade.client1_order_id_, test_order_request_sell_clone.cln_order_id_);
  BOOST_CHECK_EQUAL(client_trade.client2_, test_order_request_buy_clone.client_);
  BOOST_CHECK_EQUAL(client_trade.client2_order_id_, test_order_request_buy_clone.cln_order_id_);
}

BOOST_AUTO_TEST_CASE(LimitBuyCrossLimitSell_AggressivePrice_SingleTradeFullyFilled) {
  PriceTimePriorityMatching<> matching_engine;

  ClientOrderRequest<> test_order_request_buy{
      OrderSide::BUY,
      OrderAction::NEW,
      OrderType::LIMIT,
      GenTestOrderID(),
      DEFAULT_TEST_ORDER_SIZE,
      DEFAULT_TEST_ORDER_PRICE + 1,
      DEFAULT_TEST_CLIENT_1_ID,
      DEFAULT_TEST_INSTRUMENT_1_ID
  };

  ClientOrderRequest<> test_order_request_sell{
      OrderSide::SELL,
      OrderAction::NEW,
      OrderType::LIMIT,
      GenTestOrderID(),
      DEFAULT_TEST_ORDER_SIZE,
      DEFAULT_TEST_ORDER_PRICE,
      DEFAULT_TEST_CLIENT_1_ID,
      DEFAULT_TEST_INSTRUMENT_1_ID
  };

  const ClientOrderRequest<> test_order_request_buy_clone = test_order_request_buy;
  const ClientOrderRequest<> test_order_request_sell_clone = test_order_request_sell;

  PassiveOrderBook<> test_passive_order_book{};
  EngineEventTestObserver test_observer;

  matching_engine.doProcessOrderRequest(test_order_request_sell, test_passive_order_book, test_observer);
  matching_engine.doProcessOrderRequest(test_order_request_buy, test_passive_order_book, test_observer);

  const PriceType TRADE_PRICE = test_order_request_sell_clone.price_;

  // Expect no order queued, 1 trade
  const auto &ask_order_queue{test_passive_order_book.getAskOrderQueue()};
  BOOST_CHECK_EQUAL(ask_order_queue.size(), 0);

  const auto &bid_order_queue{test_passive_order_book.getBidOrderQueue()};
  BOOST_CHECK_EQUAL(bid_order_queue.size(), 0);

  BOOST_CHECK(test_observer.client_trade_events_.size() == 1);
  const auto &client_trade = test_observer.client_trade_events_[0];
  BOOST_CHECK_EQUAL(client_trade.trade_price_, TRADE_PRICE);
  BOOST_CHECK_EQUAL(client_trade.size_, test_order_request_buy_clone.size_);
  BOOST_CHECK_EQUAL(client_trade.client1_, test_order_request_buy_clone.client_);
  BOOST_CHECK_EQUAL(client_trade.client1_order_id_, test_order_request_buy_clone.cln_order_id_);
  BOOST_CHECK_EQUAL(client_trade.client2_, test_order_request_sell_clone.client_);
  BOOST_CHECK_EQUAL(client_trade.client2_order_id_, test_order_request_sell_clone.cln_order_id_);
}

BOOST_AUTO_TEST_CASE(LimitBuyCrossLimitSell_SingleTradePartialFilled) {
  PriceTimePriorityMatching<> matching_engine;

  ClientOrderRequest<> test_order_request_buy{
      OrderSide::BUY,
      OrderAction::NEW,
      OrderType::LIMIT,
      GenTestOrderID(),
      DEFAULT_TEST_ORDER_SIZE - 1,
      DEFAULT_TEST_ORDER_PRICE,
      DEFAULT_TEST_CLIENT_1_ID,
      DEFAULT_TEST_INSTRUMENT_1_ID
  };

  ClientOrderRequest<> test_order_request_sell{
      OrderSide::SELL,
      OrderAction::NEW,
      OrderType::LIMIT,
      GenTestOrderID(),
      DEFAULT_TEST_ORDER_SIZE,
      DEFAULT_TEST_ORDER_PRICE,
      DEFAULT_TEST_CLIENT_1_ID,
      DEFAULT_TEST_INSTRUMENT_1_ID
  };

  const ClientOrderRequest<> test_order_request_buy_clone = test_order_request_buy;
  const ClientOrderRequest<> test_order_request_sell_clone = test_order_request_sell;

  PassiveOrderBook<> test_passive_order_book{};
  EngineEventTestObserver test_observer;

  matching_engine.doProcessOrderRequest(test_order_request_sell, test_passive_order_book, test_observer);
  matching_engine.doProcessOrderRequest(test_order_request_buy, test_passive_order_book, test_observer);

  constexpr SizeType TRADED_QTY = DEFAULT_TEST_ORDER_SIZE - 1;

  // Expect remaining quantity on ask queue is 1, empty bid queue
  const auto &ask_order_queue{test_passive_order_book.getAskOrderQueue()};
  BOOST_CHECK_EQUAL(ask_order_queue.size(), 1);

  const auto &bid_order_queue{test_passive_order_book.getBidOrderQueue()};
  BOOST_CHECK_EQUAL(bid_order_queue.size(), 0);

  const auto &[best_ask_price, best_ask_order_queue] {*ask_order_queue.begin()};
  BOOST_CHECK_EQUAL(best_ask_price, test_order_request_sell_clone.price_);
  BOOST_CHECK_EQUAL(best_ask_order_queue.size(), 1);

  const auto &first_best_ask_order = best_ask_order_queue.front();
  BOOST_CHECK_EQUAL(first_best_ask_order->remaining_size_, 1);
  BOOST_CHECK_EQUAL(first_best_ask_order->cln_order_id_, test_order_request_sell.cln_order_id_);

  // Expect 1 trade
  BOOST_CHECK(test_observer.client_trade_events_.size() == 1);
  const auto &client_trade = test_observer.client_trade_events_[0];
  BOOST_CHECK_EQUAL(client_trade.trade_price_, test_order_request_buy_clone.price_);
  BOOST_CHECK_EQUAL(client_trade.size_, TRADED_QTY);
  BOOST_CHECK_EQUAL(client_trade.client1_, test_order_request_buy_clone.client_);
  BOOST_CHECK_EQUAL(client_trade.client1_order_id_, test_order_request_buy_clone.cln_order_id_);
  BOOST_CHECK_EQUAL(client_trade.client2_, test_order_request_sell_clone.client_);
  BOOST_CHECK_EQUAL(client_trade.client2_order_id_, test_order_request_sell_clone.cln_order_id_);
}

BOOST_AUTO_TEST_CASE(LimitSellCrossLimitBuy_SingleTradePartialFilled) {
  PriceTimePriorityMatching<> matching_engine;

  ClientOrderRequest<> test_order_request_buy{
      OrderSide::BUY,
      OrderAction::NEW,
      OrderType::LIMIT,
      GenTestOrderID(),
      DEFAULT_TEST_ORDER_SIZE,
      DEFAULT_TEST_ORDER_PRICE,
      DEFAULT_TEST_CLIENT_1_ID,
      DEFAULT_TEST_INSTRUMENT_1_ID
  };

  ClientOrderRequest<> test_order_request_sell{
      OrderSide::SELL,
      OrderAction::NEW,
      OrderType::LIMIT,
      GenTestOrderID(),
      DEFAULT_TEST_ORDER_SIZE - 1,
      DEFAULT_TEST_ORDER_PRICE,
      DEFAULT_TEST_CLIENT_1_ID,
      DEFAULT_TEST_INSTRUMENT_1_ID
  };

  const ClientOrderRequest<> test_order_request_buy_clone = test_order_request_buy;
  const ClientOrderRequest<> test_order_request_sell_clone = test_order_request_sell;

  PassiveOrderBook<> test_passive_order_book{};
  EngineEventTestObserver test_observer;

  matching_engine.doProcessOrderRequest(test_order_request_buy, test_passive_order_book, test_observer);
  matching_engine.doProcessOrderRequest(test_order_request_sell, test_passive_order_book, test_observer);

  constexpr SizeType TRADED_QTY = DEFAULT_TEST_ORDER_SIZE - 1;

  // Expect remaining quantity on bid queue is 1, empty ask queue
  const auto &ask_order_queue{test_passive_order_book.getAskOrderQueue()};
  BOOST_CHECK_EQUAL(ask_order_queue.size(), 0);

  const auto &bid_order_queue{test_passive_order_book.getBidOrderQueue()};
  BOOST_CHECK_EQUAL(bid_order_queue.size(), 1);

  const auto &[best_bid_price, best_bid_order_queue] {*bid_order_queue.begin()};
  BOOST_CHECK_EQUAL(best_bid_price, test_order_request_buy_clone.price_);
  BOOST_CHECK_EQUAL(best_bid_order_queue.size(), 1);

  const auto &first_best_bid_order = best_bid_order_queue.front();
  BOOST_CHECK_EQUAL(first_best_bid_order->remaining_size_, 1);
  BOOST_CHECK_EQUAL(first_best_bid_order->cln_order_id_, test_order_request_buy_clone.cln_order_id_);

  // Expect 1 trade
  BOOST_CHECK(test_observer.client_trade_events_.size() == 1);
  const auto &client_trade = test_observer.client_trade_events_[0];
  BOOST_CHECK_EQUAL(client_trade.trade_price_, test_order_request_sell_clone.price_);
  BOOST_CHECK_EQUAL(client_trade.size_, TRADED_QTY);
  BOOST_CHECK_EQUAL(client_trade.client1_, test_order_request_sell_clone.client_);
  BOOST_CHECK_EQUAL(client_trade.client1_order_id_, test_order_request_sell_clone.cln_order_id_);
  BOOST_CHECK_EQUAL(client_trade.client2_, test_order_request_buy_clone.client_);
  BOOST_CHECK_EQUAL(client_trade.client2_order_id_, test_order_request_buy_clone.cln_order_id_);
}

BOOST_AUTO_TEST_CASE(LimitBuyCrossLimitSell_FullyFilled_BuyResidualVolPlacedAsPassive) {
  constexpr SizeType UNMATCHED_VOLUME = 65;

  PriceTimePriorityMatching<> matching_engine;

  ClientOrderRequest<> test_order_request_buy{
      OrderSide::BUY,
      OrderAction::NEW,
      OrderType::LIMIT,
      GenTestOrderID(),
      DEFAULT_TEST_ORDER_SIZE + UNMATCHED_VOLUME,
      DEFAULT_TEST_ORDER_PRICE + 2,
      DEFAULT_TEST_CLIENT_1_ID,
      DEFAULT_TEST_INSTRUMENT_1_ID
  };

  ClientOrderRequest<> test_order_request_sell{
      OrderSide::SELL,
      OrderAction::NEW,
      OrderType::LIMIT,
      GenTestOrderID(),
      DEFAULT_TEST_ORDER_SIZE,
      DEFAULT_TEST_ORDER_PRICE,
      DEFAULT_TEST_CLIENT_1_ID,
      DEFAULT_TEST_INSTRUMENT_1_ID
  };

  const ClientOrderRequest<> test_order_request_buy_clone = test_order_request_buy;
  const ClientOrderRequest<> test_order_request_sell_clone = test_order_request_sell;

  PassiveOrderBook<> test_passive_order_book{};
  EngineEventTestObserver test_observer;

  matching_engine.doProcessOrderRequest(test_order_request_sell, test_passive_order_book, test_observer);
  matching_engine.doProcessOrderRequest(test_order_request_buy, test_passive_order_book, test_observer);

  constexpr SizeType TRADED_QTY = DEFAULT_TEST_ORDER_SIZE;
  constexpr PriceType TRADE_PRICE = DEFAULT_TEST_ORDER_PRICE;

  // Expect no order on ask queue, and buy order unmatched volume sitting on bid queue
  const auto &ask_order_queue{test_passive_order_book.getAskOrderQueue()};
  BOOST_CHECK_EQUAL(ask_order_queue.size(), 0);

  const auto &bid_order_queue{test_passive_order_book.getBidOrderQueue()};
  BOOST_CHECK_EQUAL(bid_order_queue.size(), 1);

  // Expect remaining quantity on queue is UNMATCHED_VOLUME
  const auto &[best_bid_price, best_bid_order_queue] {*bid_order_queue.begin()};
  BOOST_CHECK_EQUAL(best_bid_price, test_order_request_buy_clone.price_);
  BOOST_CHECK_EQUAL(best_bid_order_queue.size(), 1);

  const auto &first_best_bid_order = best_bid_order_queue.front();
  BOOST_CHECK_EQUAL(first_best_bid_order->remaining_size_, UNMATCHED_VOLUME);
  BOOST_CHECK_EQUAL(first_best_bid_order->cln_order_id_, test_order_request_buy_clone.cln_order_id_);

  // Expect 1 trade
  BOOST_CHECK(test_observer.client_trade_events_.size() == 1);
  const auto &client_trade = test_observer.client_trade_events_[0];
  BOOST_CHECK_EQUAL(client_trade.trade_price_, test_order_request_sell_clone.price_);
  BOOST_CHECK_EQUAL(client_trade.size_, TRADED_QTY);
  BOOST_CHECK_EQUAL(client_trade.client1_, test_order_request_buy_clone.client_);
  BOOST_CHECK_EQUAL(client_trade.client1_order_id_, test_order_request_buy_clone.cln_order_id_);
  BOOST_CHECK_EQUAL(client_trade.client2_, test_order_request_sell_clone.client_);
  BOOST_CHECK_EQUAL(client_trade.client2_order_id_, test_order_request_sell_clone.cln_order_id_);
}

BOOST_AUTO_TEST_CASE(LimitSellCrossLimitBuy_FullyFilled_SellResidualVolPlacedAsPassive) {
  constexpr SizeType UNMATCHED_VOLUME = 65;

  PriceTimePriorityMatching<> matching_engine;

  ClientOrderRequest<> test_order_request_buy{
      OrderSide::BUY,
      OrderAction::NEW,
      OrderType::LIMIT,
      GenTestOrderID(),
      DEFAULT_TEST_ORDER_SIZE,
      DEFAULT_TEST_ORDER_PRICE,
      DEFAULT_TEST_CLIENT_1_ID,
      DEFAULT_TEST_INSTRUMENT_1_ID
  };

  ClientOrderRequest<> test_order_request_sell{
      OrderSide::SELL,
      OrderAction::NEW,
      OrderType::LIMIT,
      GenTestOrderID(),
      DEFAULT_TEST_ORDER_SIZE + UNMATCHED_VOLUME,
      DEFAULT_TEST_ORDER_PRICE - 2,
      DEFAULT_TEST_CLIENT_1_ID,
      DEFAULT_TEST_INSTRUMENT_1_ID
  };

  const ClientOrderRequest<> test_order_request_buy_clone = test_order_request_buy;
  const ClientOrderRequest<> test_order_request_sell_clone = test_order_request_sell;

  PassiveOrderBook<> test_passive_order_book{};
  EngineEventTestObserver test_observer;

  matching_engine.doProcessOrderRequest(test_order_request_buy, test_passive_order_book, test_observer);
  matching_engine.doProcessOrderRequest(test_order_request_sell, test_passive_order_book, test_observer);

  constexpr SizeType TRADED_QTY = DEFAULT_TEST_ORDER_SIZE;
  constexpr PriceType TRADE_PRICE = DEFAULT_TEST_ORDER_PRICE;

  // Expect no order on bid queue, and sell order unmatched volume sitting on ask queue
  const auto &ask_order_queue{test_passive_order_book.getAskOrderQueue()};
  BOOST_CHECK_EQUAL(ask_order_queue.size(), 1);

  const auto &bid_order_queue{test_passive_order_book.getBidOrderQueue()};
  BOOST_CHECK_EQUAL(bid_order_queue.size(), 0);

  // Expect remaining quantity on queue is UNMATCHED_VOLUME
  const auto &[best_ask_price, best_ask_order_queue] {*ask_order_queue.begin()};
  BOOST_CHECK_EQUAL(best_ask_price, test_order_request_sell_clone.price_);
  BOOST_CHECK_EQUAL(best_ask_order_queue.size(), 1);

  const auto &first_best_ask_order = best_ask_order_queue.front();
  BOOST_CHECK_EQUAL(first_best_ask_order->remaining_size_, UNMATCHED_VOLUME);
  BOOST_CHECK_EQUAL(first_best_ask_order->cln_order_id_, test_order_request_sell_clone.cln_order_id_);

  // Expect 1 trade
  BOOST_CHECK(test_observer.client_trade_events_.size() == 1);
  const auto &client_trade = test_observer.client_trade_events_[0];
  BOOST_CHECK_EQUAL(client_trade.trade_price_, TRADE_PRICE);
  BOOST_CHECK_EQUAL(client_trade.size_, TRADED_QTY);
  BOOST_CHECK_EQUAL(client_trade.client1_, test_order_request_sell_clone.client_);
  BOOST_CHECK_EQUAL(client_trade.client1_order_id_, test_order_request_sell_clone.cln_order_id_);
  BOOST_CHECK_EQUAL(client_trade.client2_, test_order_request_buy_clone.client_);
  BOOST_CHECK_EQUAL(client_trade.client2_order_id_, test_order_request_buy_clone.cln_order_id_);
}

BOOST_AUTO_TEST_CASE(MarketBuyCrossLimitSell_SingleTradePartialFilled) {
  PriceTimePriorityMatching<> matching_engine;

  ClientOrderRequest<> test_order_request_buy{
      OrderSide::BUY,
      OrderAction::NEW,
      OrderType::MARKET,
      GenTestOrderID(),
      DEFAULT_TEST_ORDER_SIZE - 1,
      DEFAULT_TEST_ORDER_PRICE,
      DEFAULT_TEST_CLIENT_1_ID,
      DEFAULT_TEST_INSTRUMENT_1_ID
  };

  ClientOrderRequest<> test_order_request_sell{
      OrderSide::SELL,
      OrderAction::NEW,
      OrderType::LIMIT,
      GenTestOrderID(),
      DEFAULT_TEST_ORDER_SIZE,
      DEFAULT_TEST_ORDER_PRICE,
      DEFAULT_TEST_CLIENT_1_ID,
      DEFAULT_TEST_INSTRUMENT_1_ID
  };

  const ClientOrderRequest<> test_order_request_buy_clone = test_order_request_buy;
  const ClientOrderRequest<> test_order_request_sell_clone = test_order_request_sell;

  PassiveOrderBook<> test_passive_order_book{};
  EngineEventTestObserver test_observer;

  matching_engine.doProcessOrderRequest(test_order_request_sell, test_passive_order_book, test_observer);
  matching_engine.doProcessOrderRequest(test_order_request_buy, test_passive_order_book, test_observer);

  constexpr SizeType TRADED_QTY = DEFAULT_TEST_ORDER_SIZE - 1;
  constexpr SizeType RESIDUAL_QTY = 1;

  // Expect sell order remains in order queue with residual volume
  const auto &ask_order_queue{test_passive_order_book.getAskOrderQueue()};
  BOOST_CHECK_EQUAL(ask_order_queue.size(), 1);

  const auto &bid_order_queue{test_passive_order_book.getBidOrderQueue()};
  BOOST_CHECK_EQUAL(bid_order_queue.size(), 0);

  // Expect remaining quantity on queue is 1
  const auto &[best_ask_price, best_ask_order_queue] {*ask_order_queue.begin()};
  BOOST_CHECK_EQUAL(best_ask_price, test_order_request_sell_clone.price_);
  BOOST_CHECK_EQUAL(best_ask_order_queue.size(), 1);

  const auto &first_best_ask_order = best_ask_order_queue.front();
  BOOST_CHECK_EQUAL(first_best_ask_order->remaining_size_, RESIDUAL_QTY);
  BOOST_CHECK_EQUAL(first_best_ask_order->cln_order_id_, test_order_request_sell_clone.cln_order_id_);

  // Expect 1 trade
  BOOST_CHECK(test_observer.client_trade_events_.size() == 1);
  const auto &client_trade = test_observer.client_trade_events_[0];
  BOOST_CHECK_EQUAL(client_trade.trade_price_, test_order_request_sell_clone.price_);
  BOOST_CHECK_EQUAL(client_trade.size_, TRADED_QTY);
  BOOST_CHECK_EQUAL(client_trade.client1_, test_order_request_buy_clone.client_);
  BOOST_CHECK_EQUAL(client_trade.client1_order_id_, test_order_request_buy_clone.cln_order_id_);
  BOOST_CHECK_EQUAL(client_trade.client2_, test_order_request_sell_clone.client_);
  BOOST_CHECK_EQUAL(client_trade.client2_order_id_, test_order_request_sell_clone.cln_order_id_);
}

BOOST_AUTO_TEST_CASE(MarketSellCrossLimitBuy_SingleTradePartialFilled) {
  PriceTimePriorityMatching<> matching_engine;

  ClientOrderRequest<> test_order_request_buy{
      OrderSide::BUY,
      OrderAction::NEW,
      OrderType::LIMIT,
      GenTestOrderID(),
      DEFAULT_TEST_ORDER_SIZE,
      DEFAULT_TEST_ORDER_PRICE,
      DEFAULT_TEST_CLIENT_1_ID,
      DEFAULT_TEST_INSTRUMENT_1_ID
  };

  ClientOrderRequest<> test_order_request_sell{
      OrderSide::SELL,
      OrderAction::NEW,
      OrderType::MARKET,
      GenTestOrderID(),
      DEFAULT_TEST_ORDER_SIZE - 1,
      DEFAULT_TEST_ORDER_PRICE,
      DEFAULT_TEST_CLIENT_1_ID,
      DEFAULT_TEST_INSTRUMENT_1_ID
  };

  const ClientOrderRequest<> test_order_request_buy_clone = test_order_request_buy;
  const ClientOrderRequest<> test_order_request_sell_clone = test_order_request_sell;

  PassiveOrderBook<> test_passive_order_book{};
  EngineEventTestObserver test_observer;

  matching_engine.doProcessOrderRequest(test_order_request_buy, test_passive_order_book, test_observer);
  matching_engine.doProcessOrderRequest(test_order_request_sell, test_passive_order_book, test_observer);

  constexpr SizeType TRADED_QTY = DEFAULT_TEST_ORDER_SIZE - 1;
  constexpr SizeType RESIDUAL_QTY = 1;

  // Expect partial fill with the sell order remains in order queue with residual volume
  const auto &ask_order_queue{test_passive_order_book.getAskOrderQueue()};
  BOOST_CHECK_EQUAL(ask_order_queue.size(), 0);

  const auto &bid_order_queue{test_passive_order_book.getBidOrderQueue()};
  BOOST_CHECK_EQUAL(bid_order_queue.size(), 1);

  // Expect remaining quantity on queue is 1
  const auto &[best_bid_price, best_bid_order_queue] {*bid_order_queue.begin()};
  BOOST_CHECK_EQUAL(best_bid_price, test_order_request_buy.price_);
  BOOST_CHECK_EQUAL(best_bid_order_queue.size(), 1);

  const auto &first_best_bid_order = best_bid_order_queue.front();
  BOOST_CHECK_EQUAL(first_best_bid_order->remaining_size_, RESIDUAL_QTY);
  BOOST_CHECK_EQUAL(first_best_bid_order->cln_order_id_, test_order_request_buy_clone.cln_order_id_);

  // Expect 1 trade
  BOOST_CHECK(test_observer.client_trade_events_.size() == 1);
  const auto &client_trade = test_observer.client_trade_events_[0];
  BOOST_CHECK_EQUAL(client_trade.trade_price_, test_order_request_sell_clone.price_);
  BOOST_CHECK_EQUAL(client_trade.size_, TRADED_QTY);
  BOOST_CHECK_EQUAL(client_trade.client1_, test_order_request_sell_clone.client_);
  BOOST_CHECK_EQUAL(client_trade.client1_order_id_, test_order_request_sell_clone.cln_order_id_);
  BOOST_CHECK_EQUAL(client_trade.client2_, test_order_request_buy_clone.client_);
  BOOST_CHECK_EQUAL(client_trade.client2_order_id_, test_order_request_buy_clone.cln_order_id_);
}

BOOST_AUTO_TEST_CASE(LimitSellCrossMultipleLimitBuy_MultipleTradesOnMultiplePriceLevels) {
  PriceTimePriorityMatching<> matching_engine;

  ClientOrderRequest<> test_order_request_buy1{
      OrderSide::BUY,
      OrderAction::NEW,
      OrderType::LIMIT,
      GenTestOrderID(),
      DEFAULT_TEST_ORDER_SIZE,
      DEFAULT_TEST_ORDER_PRICE,
      DEFAULT_TEST_CLIENT_1_ID,
      DEFAULT_TEST_INSTRUMENT_1_ID
  };

  ClientOrderRequest<> test_order_request_buy2{
      OrderSide::BUY,
      OrderAction::NEW,
      OrderType::LIMIT,
      GenTestOrderID(),
      DEFAULT_TEST_ORDER_SIZE,
      DEFAULT_TEST_ORDER_PRICE,
      DEFAULT_TEST_CLIENT_2_ID,
      DEFAULT_TEST_INSTRUMENT_1_ID
  };

  ClientOrderRequest<> test_order_request_buy3{
      OrderSide::BUY,
      OrderAction::NEW,
      OrderType::LIMIT,
      GenTestOrderID(),
      DEFAULT_TEST_ORDER_SIZE,
      DEFAULT_TEST_ORDER_PRICE - 1,
      DEFAULT_TEST_CLIENT_3_ID,
      DEFAULT_TEST_INSTRUMENT_1_ID
  };

  ClientOrderRequest<> test_order_request_buy4{
      OrderSide::BUY,
      OrderAction::NEW,
      OrderType::LIMIT,
      GenTestOrderID(),
      DEFAULT_TEST_ORDER_SIZE,
      DEFAULT_TEST_ORDER_PRICE - 1,
      DEFAULT_TEST_CLIENT_4_ID,
      DEFAULT_TEST_INSTRUMENT_1_ID
  };

  ClientOrderRequest<> test_order_request_buy5{
      OrderSide::BUY,
      OrderAction::NEW,
      OrderType::LIMIT,
      GenTestOrderID(),
      DEFAULT_TEST_ORDER_SIZE,
      DEFAULT_TEST_ORDER_PRICE - 2,
      DEFAULT_TEST_CLIENT_5_ID,
      DEFAULT_TEST_INSTRUMENT_1_ID
  };

  ClientOrderRequest<> test_order_request_sell{
      OrderSide::SELL,
      OrderAction::NEW,
      OrderType::LIMIT,
      GenTestOrderID(),
      DEFAULT_TEST_ORDER_SIZE * 10,
      DEFAULT_TEST_ORDER_PRICE - 1,
      DEFAULT_TEST_CLIENT_6_ID,
      DEFAULT_TEST_INSTRUMENT_1_ID
  };

  const ClientOrderRequest<> test_order_request_buy1_clone = test_order_request_buy1;
  const ClientOrderRequest<> test_order_request_buy2_clone = test_order_request_buy2;
  const ClientOrderRequest<> test_order_request_buy3_clone = test_order_request_buy3;
  const ClientOrderRequest<> test_order_request_buy4_clone = test_order_request_buy4;
  const ClientOrderRequest<> test_order_request_buy5_clone = test_order_request_buy5;
  const ClientOrderRequest<> test_order_request_sell_clone = test_order_request_sell;

  PassiveOrderBook<> test_passive_order_book{};
  EngineEventTestObserver test_observer;

  matching_engine.doProcessOrderRequest(test_order_request_buy1, test_passive_order_book, test_observer);
  matching_engine.doProcessOrderRequest(test_order_request_buy2, test_passive_order_book, test_observer);
  matching_engine.doProcessOrderRequest(test_order_request_buy3, test_passive_order_book, test_observer);
  matching_engine.doProcessOrderRequest(test_order_request_buy4, test_passive_order_book, test_observer);
  matching_engine.doProcessOrderRequest(test_order_request_buy5, test_passive_order_book, test_observer);
  matching_engine.doProcessOrderRequest(test_order_request_sell, test_passive_order_book, test_observer);

  // Expect all test_order_request_buy[1..4] are fully executed
  const auto &ask_order_queue{test_passive_order_book.getAskOrderQueue()};
  BOOST_CHECK_EQUAL(ask_order_queue.size(), 1);

  const auto &bid_order_queue{test_passive_order_book.getBidOrderQueue()};
  BOOST_CHECK_EQUAL(bid_order_queue.size(), 1);

  const auto &[best_bid_price, best_bid_order_queue] {*bid_order_queue.begin()};
  BOOST_CHECK_EQUAL(best_bid_price, test_order_request_buy5_clone.price_);
  BOOST_CHECK_EQUAL(best_bid_order_queue.size(), 1);

  const auto &first_best_bid_order = best_bid_order_queue.front();
  BOOST_CHECK_EQUAL(first_best_bid_order->remaining_size_, test_order_request_buy5_clone.size_);
  BOOST_CHECK_EQUAL(first_best_bid_order->cln_order_id_, test_order_request_buy5_clone.cln_order_id_);

  const auto &[best_ask_price, best_ask_order_queue] {*ask_order_queue.begin()};
  BOOST_CHECK_EQUAL(best_ask_price, test_order_request_sell_clone.price_);
  BOOST_CHECK_EQUAL(best_ask_order_queue.size(), 1);

  const SizeType SELL_ORDER_RESIDUAL_VOLUME =
      test_order_request_sell_clone.size_ - test_order_request_buy1_clone.size_ - test_order_request_buy2_clone.size_
          - test_order_request_buy3_clone.size_ - test_order_request_buy4_clone.size_;

  const auto &first_best_ask_order = best_ask_order_queue.front();
  BOOST_CHECK_EQUAL(first_best_ask_order->remaining_size_, SELL_ORDER_RESIDUAL_VOLUME);
  BOOST_CHECK_EQUAL(first_best_ask_order->cln_order_id_, test_order_request_sell_clone.cln_order_id_);

  // Expect 4 trades
  BOOST_CHECK(test_observer.client_trade_events_.size() == 4);

  // Trade 1
  const auto &client_trade1 = test_observer.client_trade_events_[0];
  BOOST_CHECK_EQUAL(client_trade1.trade_price_, test_order_request_buy1_clone.price_);
  BOOST_CHECK_EQUAL(client_trade1.size_, DEFAULT_TEST_ORDER_SIZE);
  BOOST_CHECK_EQUAL(client_trade1.client1_, test_order_request_sell_clone.client_);
  BOOST_CHECK_EQUAL(client_trade1.client1_order_id_, test_order_request_sell_clone.cln_order_id_);
  BOOST_CHECK_EQUAL(client_trade1.client2_, test_order_request_buy1_clone.client_);
  BOOST_CHECK_EQUAL(client_trade1.client2_order_id_, test_order_request_buy1_clone.cln_order_id_);

  // Trade 2
  const auto &client_trade2 = test_observer.client_trade_events_[1];
  BOOST_CHECK_EQUAL(client_trade2.trade_price_, test_order_request_buy2_clone.price_);
  BOOST_CHECK_EQUAL(client_trade2.size_, DEFAULT_TEST_ORDER_SIZE);
  BOOST_CHECK_EQUAL(client_trade2.client1_, test_order_request_sell_clone.client_);
  BOOST_CHECK_EQUAL(client_trade2.client1_order_id_, test_order_request_sell_clone.cln_order_id_);
  BOOST_CHECK_EQUAL(client_trade2.client2_, test_order_request_buy2_clone.client_);
  BOOST_CHECK_EQUAL(client_trade2.client2_order_id_, test_order_request_buy2_clone.cln_order_id_);

  // Trade 3
  const auto &client_trade3 = test_observer.client_trade_events_[2];
  BOOST_CHECK_EQUAL(client_trade3.trade_price_, test_order_request_buy3_clone.price_);
  BOOST_CHECK_EQUAL(client_trade3.size_, DEFAULT_TEST_ORDER_SIZE);
  BOOST_CHECK_EQUAL(client_trade3.client1_, test_order_request_sell_clone.client_);
  BOOST_CHECK_EQUAL(client_trade3.client1_order_id_, test_order_request_sell_clone.cln_order_id_);
  BOOST_CHECK_EQUAL(client_trade3.client2_, test_order_request_buy3_clone.client_);
  BOOST_CHECK_EQUAL(client_trade3.client2_order_id_, test_order_request_buy3_clone.cln_order_id_);

  // Trade 4
  const auto &client_trade4 = test_observer.client_trade_events_[3];
  BOOST_CHECK_EQUAL(client_trade4.trade_price_, test_order_request_buy4_clone.price_);
  BOOST_CHECK_EQUAL(client_trade4.size_, DEFAULT_TEST_ORDER_SIZE);
  BOOST_CHECK_EQUAL(client_trade4.client1_, test_order_request_sell_clone.client_);
  BOOST_CHECK_EQUAL(client_trade4.client1_order_id_, test_order_request_sell_clone.cln_order_id_);
  BOOST_CHECK_EQUAL(client_trade4.client2_, test_order_request_buy4_clone.client_);
  BOOST_CHECK_EQUAL(client_trade4.client2_order_id_, test_order_request_buy4_clone.cln_order_id_);
}

BOOST_AUTO_TEST_CASE(LimitBuyCrossMultipleLimitSell_MultipleTradesOnMultiplePriceLevels) {
  PriceTimePriorityMatching<> matching_engine;

  ClientOrderRequest<> test_order_request_sell1{
      OrderSide::SELL,
      OrderAction::NEW,
      OrderType::LIMIT,
      GenTestOrderID(),
      DEFAULT_TEST_ORDER_SIZE,
      DEFAULT_TEST_ORDER_PRICE,
      DEFAULT_TEST_CLIENT_1_ID,
      DEFAULT_TEST_INSTRUMENT_1_ID
  };

  ClientOrderRequest<> test_order_request_sell2{
      OrderSide::SELL,
      OrderAction::NEW,
      OrderType::LIMIT,
      GenTestOrderID(),
      DEFAULT_TEST_ORDER_SIZE,
      DEFAULT_TEST_ORDER_PRICE,
      DEFAULT_TEST_CLIENT_2_ID,
      DEFAULT_TEST_INSTRUMENT_1_ID
  };

  ClientOrderRequest<> test_order_request_sell3{
      OrderSide::SELL,
      OrderAction::NEW,
      OrderType::LIMIT,
      GenTestOrderID(),
      DEFAULT_TEST_ORDER_SIZE,
      DEFAULT_TEST_ORDER_PRICE + 1,
      DEFAULT_TEST_CLIENT_3_ID,
      DEFAULT_TEST_INSTRUMENT_1_ID
  };

  ClientOrderRequest<> test_order_request_sell4{
      OrderSide::SELL,
      OrderAction::NEW,
      OrderType::LIMIT,
      GenTestOrderID(),
      DEFAULT_TEST_ORDER_SIZE,
      DEFAULT_TEST_ORDER_PRICE + 1,
      DEFAULT_TEST_CLIENT_4_ID,
      DEFAULT_TEST_INSTRUMENT_1_ID
  };

  ClientOrderRequest<> test_order_request_sell5{
      OrderSide::SELL,
      OrderAction::NEW,
      OrderType::LIMIT,
      GenTestOrderID(),
      DEFAULT_TEST_ORDER_SIZE,
      DEFAULT_TEST_ORDER_PRICE + 2,
      DEFAULT_TEST_CLIENT_5_ID,
      DEFAULT_TEST_INSTRUMENT_1_ID
  };

  ClientOrderRequest<> test_order_request_buy{
      OrderSide::BUY,
      OrderAction::NEW,
      OrderType::LIMIT,
      GenTestOrderID(),
      DEFAULT_TEST_ORDER_SIZE * 10,
      DEFAULT_TEST_ORDER_PRICE + 1,
      DEFAULT_TEST_CLIENT_6_ID,
      DEFAULT_TEST_INSTRUMENT_1_ID
  };

  const ClientOrderRequest<> test_order_request_buy_clone = test_order_request_buy;
  const ClientOrderRequest<> test_order_request_sell1_clone = test_order_request_sell1;
  const ClientOrderRequest<> test_order_request_sell2_clone = test_order_request_sell2;
  const ClientOrderRequest<> test_order_request_sell3_clone = test_order_request_sell3;
  const ClientOrderRequest<> test_order_request_sell4_clone = test_order_request_sell4;
  const ClientOrderRequest<> test_order_request_sell5_clone = test_order_request_sell5;

  PassiveOrderBook<> test_passive_order_book{};
  EngineEventTestObserver test_observer;

  matching_engine.doProcessOrderRequest(test_order_request_sell1, test_passive_order_book, test_observer);
  matching_engine.doProcessOrderRequest(test_order_request_sell2, test_passive_order_book, test_observer);
  matching_engine.doProcessOrderRequest(test_order_request_sell3, test_passive_order_book, test_observer);
  matching_engine.doProcessOrderRequest(test_order_request_sell4, test_passive_order_book, test_observer);
  matching_engine.doProcessOrderRequest(test_order_request_sell5, test_passive_order_book, test_observer);
  matching_engine.doProcessOrderRequest(test_order_request_buy, test_passive_order_book, test_observer);

  // Expect all test_order_request_sell[1..4] are fully executed
  const auto &ask_order_queue{test_passive_order_book.getAskOrderQueue()};
  BOOST_CHECK_EQUAL(ask_order_queue.size(), 1);

  const auto &bid_order_queue{test_passive_order_book.getBidOrderQueue()};
  BOOST_CHECK_EQUAL(bid_order_queue.size(), 1);

  const auto &[best_ask_price, best_ask_order_queue] {*ask_order_queue.begin()};
  BOOST_CHECK_EQUAL(best_ask_price, test_order_request_sell5_clone.price_);
  BOOST_CHECK_EQUAL(best_ask_order_queue.size(), 1);

  const auto &first_best_ask_order = best_ask_order_queue.front();
  BOOST_CHECK_EQUAL(first_best_ask_order->remaining_size_, test_order_request_sell5_clone.size_);
  BOOST_CHECK_EQUAL(first_best_ask_order->cln_order_id_, test_order_request_sell5_clone.cln_order_id_);

  const auto &[best_bid_price, best_bid_order_queue] {*bid_order_queue.begin()};
  BOOST_CHECK_EQUAL(best_bid_price, test_order_request_buy_clone.price_);
  BOOST_CHECK_EQUAL(best_bid_order_queue.size(), 1);

  const SizeType SELL_ORDER_RESIDUAL_VOLUME =
      test_order_request_buy_clone.size_ - test_order_request_sell1_clone.size_ - test_order_request_sell2_clone.size_
          - test_order_request_sell3_clone.size_ - test_order_request_sell4_clone.size_;

  const auto &first_best_bid_order = best_bid_order_queue.front();
  BOOST_CHECK_EQUAL(first_best_bid_order->remaining_size_, SELL_ORDER_RESIDUAL_VOLUME);
  BOOST_CHECK_EQUAL(first_best_bid_order->cln_order_id_, test_order_request_buy_clone.cln_order_id_);

  // Expect 4 trades
  BOOST_CHECK(test_observer.client_trade_events_.size() == 4);

  // Trade 1
  const auto &client_trade1 = test_observer.client_trade_events_[0];
  BOOST_CHECK_EQUAL(client_trade1.trade_price_, test_order_request_sell1_clone.price_);
  BOOST_CHECK_EQUAL(client_trade1.size_, DEFAULT_TEST_ORDER_SIZE);
  BOOST_CHECK_EQUAL(client_trade1.client1_, test_order_request_buy_clone.client_);
  BOOST_CHECK_EQUAL(client_trade1.client1_order_id_, test_order_request_buy_clone.cln_order_id_);
  BOOST_CHECK_EQUAL(client_trade1.client2_, test_order_request_sell1_clone.client_);
  BOOST_CHECK_EQUAL(client_trade1.client2_order_id_, test_order_request_sell1_clone.cln_order_id_);

  // Trade 2
  const auto &client_trade2 = test_observer.client_trade_events_[1];
  BOOST_CHECK_EQUAL(client_trade2.trade_price_, test_order_request_sell2_clone.price_);
  BOOST_CHECK_EQUAL(client_trade2.size_, DEFAULT_TEST_ORDER_SIZE);
  BOOST_CHECK_EQUAL(client_trade2.client1_, test_order_request_buy_clone.client_);
  BOOST_CHECK_EQUAL(client_trade2.client1_order_id_, test_order_request_buy_clone.cln_order_id_);
  BOOST_CHECK_EQUAL(client_trade2.client2_, test_order_request_sell2_clone.client_);
  BOOST_CHECK_EQUAL(client_trade2.client2_order_id_, test_order_request_sell2_clone.cln_order_id_);

  // Trade 3
  const auto &client_trade3 = test_observer.client_trade_events_[2];
  BOOST_CHECK_EQUAL(client_trade3.trade_price_, test_order_request_sell3_clone.price_);
  BOOST_CHECK_EQUAL(client_trade3.size_, DEFAULT_TEST_ORDER_SIZE);
  BOOST_CHECK_EQUAL(client_trade3.client1_, test_order_request_buy_clone.client_);
  BOOST_CHECK_EQUAL(client_trade3.client1_order_id_, test_order_request_buy_clone.cln_order_id_);
  BOOST_CHECK_EQUAL(client_trade3.client2_, test_order_request_sell3_clone.client_);
  BOOST_CHECK_EQUAL(client_trade3.client2_order_id_, test_order_request_sell3_clone.cln_order_id_);

  // Trade 4
  const auto &client_trade4 = test_observer.client_trade_events_[3];
  BOOST_CHECK_EQUAL(client_trade4.trade_price_, test_order_request_sell4_clone.price_);
  BOOST_CHECK_EQUAL(client_trade4.size_, DEFAULT_TEST_ORDER_SIZE);
  BOOST_CHECK_EQUAL(client_trade4.client1_, test_order_request_buy_clone.client_);
  BOOST_CHECK_EQUAL(client_trade4.client1_order_id_, test_order_request_buy_clone.cln_order_id_);
  BOOST_CHECK_EQUAL(client_trade4.client2_, test_order_request_sell4_clone.client_);
  BOOST_CHECK_EQUAL(client_trade4.client2_order_id_, test_order_request_sell4_clone.cln_order_id_);
}

} // end of namespace

BOOST_AUTO_TEST_SUITE_END()