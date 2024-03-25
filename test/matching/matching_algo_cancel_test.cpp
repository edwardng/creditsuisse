#include "test_helper.h"

#include <boost/test/unit_test.hpp>

#include "matching/matching_algo.hpp"
#include "events/client_order_request.h"
#include "matching/validators/cancel_request_validators.hpp"

using namespace codetest::matching_engine_sim;
using namespace codetest::matching_engine_sim_test_helper;

BOOST_AUTO_TEST_SUITE(PriceTimePriorityMatching_Cancel_TestSuite)

namespace codetest::matching_engine_sim_test {

BOOST_AUTO_TEST_CASE(InsertCancel_QueueSizeZero)
{
  using NoValidator = Validators<void>;
  PriceTimePriorityMatching<> matching_engine;

  const auto test_order_id = GenTestOrderID();

  ClientOrderRequest<> test_order_request_buy{
      OrderSide::BUY,
      OrderAction::NEW,
      OrderType::LIMIT,
      test_order_id,
      DEFAULT_TEST_ORDER_SIZE,
      DEFAULT_TEST_ORDER_PRICE,
      DEFAULT_TEST_CLIENT_1_ID,
      DEFAULT_TEST_INSTRUMENT_1_ID
  };

  ClientOrderRequest<> test_order_request_cancel{
      OrderSide::BUY,
      OrderAction::CANCEL,
      OrderType::LIMIT,
      test_order_id,
      DEFAULT_TEST_ORDER_SIZE,
      DEFAULT_TEST_ORDER_PRICE,
      DEFAULT_TEST_CLIENT_1_ID,
      DEFAULT_TEST_INSTRUMENT_1_ID
  };

  const ClientOrderRequest<> test_order_request_buy_clone = test_order_request_buy;

  PassiveOrderBook<> test_passive_order_book{};
  EngineEventTestObserver test_observer;

  matching_engine.doProcessOrderRequest(test_order_request_buy, test_passive_order_book, test_observer);
  matching_engine.doProcessOrderRequest(test_order_request_cancel, test_passive_order_book, test_observer);

  // Expect the buy order remains on bid queue with size 0
  const auto &bid_order_queue{test_passive_order_book.getBidOrderQueue()};
  BOOST_CHECK_EQUAL(bid_order_queue.size(), 1);

  auto first_bid_level_itr = bid_order_queue.begin();
  BOOST_CHECK(first_bid_level_itr != bid_order_queue.end());

  BOOST_CHECK_EQUAL(first_bid_level_itr->first, test_order_request_buy_clone.price_);

  const auto &persisted_passive_order = first_bid_level_itr->second.front();
  BOOST_CHECK_EQUAL(persisted_passive_order->cln_order_id_, test_order_request_buy_clone.cln_order_id_);
  BOOST_CHECK_EQUAL(persisted_passive_order->client_, test_order_request_buy_clone.client_);
  BOOST_CHECK_EQUAL(persisted_passive_order->remaining_size_, 0);
}

BOOST_AUTO_TEST_CASE(CancelOrderThatDoesNotExists)
{
  PriceTimePriorityMatching<> matching_engine;

  const auto test_order_id = GenTestOrderID();

  ClientOrderRequest<> test_order_request_cancel{
      OrderSide::BUY,
      OrderAction::CANCEL,
      OrderType::LIMIT,
      test_order_id,
      DEFAULT_TEST_ORDER_SIZE,
      DEFAULT_TEST_ORDER_PRICE,
      DEFAULT_TEST_CLIENT_1_ID,
      DEFAULT_TEST_INSTRUMENT_1_ID
  };

  PassiveOrderBook<> test_passive_order_book{};
  EngineEventTestObserver test_observer;

  matching_engine.doProcessOrderRequest(test_order_request_cancel, test_passive_order_book, test_observer);

  // Expect no order in queue
  const auto &bid_order_queue{test_passive_order_book.getBidOrderQueue()};
  BOOST_CHECK_EQUAL(bid_order_queue.size(), 0);

  const auto &ask_order_queue{test_passive_order_book.getAskOrderQueue()};
  BOOST_CHECK_EQUAL(ask_order_queue.size(), 0);
}

BOOST_AUTO_TEST_CASE(CancelledOrderEliminationOnMatching)
{
  PriceTimePriorityMatching<> matching_engine;

  const auto test_order_id_buy1 = GenTestOrderID();
  const auto test_order_id_buy2 = GenTestOrderID();
  const auto test_order_id_buy3 = GenTestOrderID();
  const auto test_order_id_buy4 = GenTestOrderID();
  const auto test_order_id_sell = GenTestOrderID();

  ClientOrderRequest<> test_order_request_buy1{
      OrderSide::BUY,
      OrderAction::NEW,
      OrderType::LIMIT,
      test_order_id_buy1,
      DEFAULT_TEST_ORDER_SIZE,
      DEFAULT_TEST_ORDER_PRICE,
      DEFAULT_TEST_CLIENT_1_ID,
      DEFAULT_TEST_INSTRUMENT_1_ID
  };

  ClientOrderRequest<> test_order_request_buy2{
      OrderSide::BUY,
      OrderAction::NEW,
      OrderType::LIMIT,
      test_order_id_buy2,
      DEFAULT_TEST_ORDER_SIZE,
      DEFAULT_TEST_ORDER_PRICE,
      DEFAULT_TEST_CLIENT_2_ID,
      DEFAULT_TEST_INSTRUMENT_1_ID
  };

  ClientOrderRequest<> test_order_request_buy3{
      OrderSide::BUY,
      OrderAction::NEW,
      OrderType::LIMIT,
      test_order_id_buy3,
      DEFAULT_TEST_ORDER_SIZE,
      DEFAULT_TEST_ORDER_PRICE,
      DEFAULT_TEST_CLIENT_3_ID,
      DEFAULT_TEST_INSTRUMENT_1_ID
  };

  ClientOrderRequest<> test_order_request_buy4{
      OrderSide::BUY,
      OrderAction::NEW,
      OrderType::LIMIT,
      test_order_id_buy4,
      DEFAULT_TEST_ORDER_SIZE,
      DEFAULT_TEST_ORDER_PRICE,
      DEFAULT_TEST_CLIENT_4_ID,
      DEFAULT_TEST_INSTRUMENT_1_ID
  };

  ClientOrderRequest<> test_order_request_cancel_buy2{
      OrderSide::BUY,
      OrderAction::CANCEL,
      OrderType::LIMIT,
      test_order_id_buy2,
      DEFAULT_TEST_ORDER_SIZE,
      DEFAULT_TEST_ORDER_PRICE,
      DEFAULT_TEST_CLIENT_2_ID,
      DEFAULT_TEST_INSTRUMENT_1_ID
  };

  const ClientOrderRequest<> test_order_request_buy1_clone = test_order_request_buy1;
  const ClientOrderRequest<> test_order_request_buy2_clone = test_order_request_buy2;
  const ClientOrderRequest<> test_order_request_buy3_clone = test_order_request_buy3;
  const ClientOrderRequest<> test_order_request_buy4_clone = test_order_request_buy4;

  PassiveOrderBook<> test_passive_order_book{};
  EngineEventTestObserver test_observer;

  matching_engine.doProcessOrderRequest(test_order_request_buy1, test_passive_order_book, test_observer);
  matching_engine.doProcessOrderRequest(test_order_request_buy2, test_passive_order_book, test_observer);
  matching_engine.doProcessOrderRequest(test_order_request_buy3, test_passive_order_book, test_observer);
  matching_engine.doProcessOrderRequest(test_order_request_buy4, test_passive_order_book, test_observer);
  matching_engine.doProcessOrderRequest(test_order_request_cancel_buy2, test_passive_order_book, test_observer);

  const auto &ask_order_queue{test_passive_order_book.getAskOrderQueue()};
  BOOST_CHECK_EQUAL(ask_order_queue.size(), 0);

  const auto &bid_order_queue{test_passive_order_book.getBidOrderQueue()};
  BOOST_CHECK_EQUAL(bid_order_queue.size(), 1);

  const auto &[best_bid_price, best_bid_order_queue] {*bid_order_queue.begin()};
  BOOST_CHECK_EQUAL(best_bid_order_queue.size(), 4);

  ClientOrderRequest<> test_order_request_sell{
      OrderSide::SELL,
      OrderAction::NEW,
      OrderType::LIMIT,
      test_order_id_sell,
      DEFAULT_TEST_ORDER_SIZE * 2,
      DEFAULT_TEST_ORDER_PRICE,
      DEFAULT_TEST_CLIENT_5_ID,
      DEFAULT_TEST_INSTRUMENT_1_ID
  };

  const ClientOrderRequest<> test_order_request_sell_clone = test_order_request_sell;

  matching_engine.doProcessOrderRequest(test_order_request_sell, test_passive_order_book, test_observer);

  // Expect no more order on both bid/ask queue
  BOOST_CHECK_EQUAL(bid_order_queue.size(), 1);
  BOOST_CHECK_EQUAL(ask_order_queue.size(), 0);

  BOOST_CHECK_EQUAL(best_bid_order_queue.size(), 1);
  const auto &best_bid_order = best_bid_order_queue.front();
  BOOST_CHECK_EQUAL(best_bid_order->client_, DEFAULT_TEST_CLIENT_4_ID);
  BOOST_CHECK_EQUAL(best_bid_order->cln_order_id_, test_order_id_buy4);
  BOOST_CHECK_EQUAL(best_bid_order->remaining_size_, DEFAULT_TEST_ORDER_SIZE);

  // Expect 2 trades
  BOOST_CHECK_EQUAL(test_observer.client_trade_events_.size(), 2);

  // Trade 1
  const auto &client_trade1 = test_observer.client_trade_events_[0];
  BOOST_CHECK_EQUAL(client_trade1.trade_price_, test_order_request_sell_clone.price_);
  BOOST_CHECK_EQUAL(client_trade1.size_, DEFAULT_TEST_ORDER_SIZE);
  BOOST_CHECK_EQUAL(client_trade1.client1_, test_order_request_sell_clone.client_);
  BOOST_CHECK_EQUAL(client_trade1.client1_order_id_, test_order_request_sell_clone.cln_order_id_);
  BOOST_CHECK_EQUAL(client_trade1.client2_, test_order_request_buy1_clone.client_);
  BOOST_CHECK_EQUAL(client_trade1.client2_order_id_, test_order_request_buy1_clone.cln_order_id_);

  // Trade 2
  const auto &client_trade2 = test_observer.client_trade_events_[1];
  BOOST_CHECK_EQUAL(client_trade2.trade_price_, test_order_request_sell_clone.price_);
  BOOST_CHECK_EQUAL(client_trade2.size_, DEFAULT_TEST_ORDER_SIZE);
  BOOST_CHECK_EQUAL(client_trade2.client1_, test_order_request_sell_clone.client_);
  BOOST_CHECK_EQUAL(client_trade2.client1_order_id_, test_order_request_sell_clone.cln_order_id_);
  BOOST_CHECK_EQUAL(client_trade2.client2_, test_order_request_buy3_clone.client_);
  BOOST_CHECK_EQUAL(client_trade2.client2_order_id_, test_order_request_buy3_clone.cln_order_id_);
}

BOOST_AUTO_TEST_CASE(AllowReuseCancelledClientOrderID)
{
  PriceTimePriorityMatching<> matching_engine;

  const auto test_order_id = GenTestOrderID();

  ClientOrderRequest<> test_order_request{
      OrderSide::BUY,
      OrderAction::NEW,
      OrderType::LIMIT,
      test_order_id,
      DEFAULT_TEST_ORDER_SIZE,
      DEFAULT_TEST_ORDER_PRICE,
      DEFAULT_TEST_CLIENT_1_ID,
      DEFAULT_TEST_INSTRUMENT_1_ID
  };

  ClientOrderRequest<> test_order_request_cancel{
      OrderSide::BUY,
      OrderAction::CANCEL,
      OrderType::LIMIT,
      test_order_id,
      DEFAULT_TEST_ORDER_SIZE,
      DEFAULT_TEST_ORDER_PRICE,
      DEFAULT_TEST_CLIENT_1_ID,
      DEFAULT_TEST_INSTRUMENT_1_ID
  };

  PassiveOrderBook<> test_passive_order_book{};
  EngineEventTestObserver test_observer;

  matching_engine.doProcessOrderRequest(test_order_request, test_passive_order_book, test_observer);
  matching_engine.doProcessOrderRequest(test_order_request_cancel, test_passive_order_book, test_observer);
  matching_engine.doProcessOrderRequest(test_order_request, test_passive_order_book, test_observer);

  // Expect the buy order placed on bid queue, and empty ask queue
  const auto &ask_order_queue{test_passive_order_book.getAskOrderQueue()};
  BOOST_CHECK_EQUAL(ask_order_queue.size(), 0);

  const auto &bid_order_queue{test_passive_order_book.getBidOrderQueue()};
  BOOST_CHECK_EQUAL(bid_order_queue.size(), 1);

  auto first_bid_level_itr = bid_order_queue.begin();
  BOOST_CHECK(first_bid_level_itr != bid_order_queue.end());

  const auto &[best_bid_price, best_bid_order_queue] {*bid_order_queue.begin()};
  BOOST_CHECK_EQUAL(best_bid_order_queue.size(), 2);

  const auto &first_order = best_bid_order_queue.front();
  BOOST_CHECK_EQUAL(first_order->client_, DEFAULT_TEST_CLIENT_1_ID);
  BOOST_CHECK_EQUAL(first_order->cln_order_id_, test_order_id);
  BOOST_CHECK_EQUAL(first_order->remaining_size_, 0);

  const auto &last_order = best_bid_order_queue.back();
  BOOST_CHECK_EQUAL(last_order->client_, DEFAULT_TEST_CLIENT_1_ID);
  BOOST_CHECK_EQUAL(last_order->cln_order_id_, test_order_id);
  BOOST_CHECK_EQUAL(last_order->remaining_size_, DEFAULT_TEST_ORDER_SIZE);
}

BOOST_AUTO_TEST_CASE(NoSuchOrderValidation_InsertCancel)
{
  using OrderExt = void;
  using MatchValidators = Validators<OrderExt>;
  using InsertValidators = Validators<OrderExt>;
  using CancelValidators = Validators<OrderExt, NoSuchOrderCancelValidator<OrderExt>>;

  PriceTimePriorityMatching<OrderExt, MatchValidators, InsertValidators, CancelValidators>
      matching_engine;

  const auto test_order_id = GenTestOrderID();

  ClientOrderRequest<> test_order_request_buy{
      OrderSide::BUY,
      OrderAction::NEW,
      OrderType::LIMIT,
      test_order_id,
      DEFAULT_TEST_ORDER_SIZE,
      DEFAULT_TEST_ORDER_PRICE,
      DEFAULT_TEST_CLIENT_1_ID,
      DEFAULT_TEST_INSTRUMENT_1_ID
  };

  ClientOrderRequest<> test_order_request_cancel{
      OrderSide::BUY,
      OrderAction::CANCEL,
      OrderType::LIMIT,
      test_order_id,
      DEFAULT_TEST_ORDER_SIZE,
      DEFAULT_TEST_ORDER_PRICE,
      DEFAULT_TEST_CLIENT_1_ID,
      DEFAULT_TEST_INSTRUMENT_1_ID
  };

  const ClientOrderRequest<> test_order_request_buy_clone = test_order_request_buy;
  const ClientOrderRequest<> test_order_request_cancel_clone = test_order_request_cancel;

  PassiveOrderBook<> test_passive_order_book{};
  EngineEventTestObserver test_observer;

  matching_engine.doProcessOrderRequest(test_order_request_buy, test_passive_order_book, test_observer);
  matching_engine.doProcessOrderRequest(test_order_request_cancel, test_passive_order_book, test_observer);

  const auto &order_responses = test_observer.client_order_responses_;
  BOOST_CHECK_EQUAL(order_responses.size(), 2);
  const auto &order_response1 = order_responses[0];
  BOOST_CHECK_EQUAL(order_response1.client_, test_order_request_buy_clone.client_);
  BOOST_CHECK_EQUAL(order_response1.client_order_id_, test_order_request_buy_clone.cln_order_id_);
  BOOST_CHECK_EQUAL(order_response1.size_, test_order_request_buy.size_);
  BOOST_CHECK_EQUAL(order_response1.order_price_, test_order_request_buy.price_);
  BOOST_CHECK_EQUAL(order_response1.instrument_, test_order_request_buy.instrument_);
  BOOST_CHECK(order_response1.validation_response_ == ValidationResponse::NO_ERROR);
  BOOST_CHECK(order_response1.request_result_ == OrderRequestResult::ACK);

  const auto &order_response2 = order_responses[1];
  BOOST_CHECK_EQUAL(order_response2.client_, test_order_request_buy_clone.client_);
  BOOST_CHECK_EQUAL(order_response2.client_order_id_, test_order_request_buy_clone.cln_order_id_);
  BOOST_CHECK_EQUAL(order_response2.size_, test_order_request_buy.size_);
  BOOST_CHECK_EQUAL(order_response2.order_price_, test_order_request_buy.price_);
  BOOST_CHECK_EQUAL(order_response2.instrument_, test_order_request_buy.instrument_);
  BOOST_CHECK(order_response2.validation_response_ == ValidationResponse::NO_ERROR);
  BOOST_CHECK(order_response2.request_result_ == OrderRequestResult::ACK);
}

BOOST_AUTO_TEST_CASE(NoSuchOrderValidation_CancelNotExistOrde)
{
  using OrderExt = void;
  using MatchValidators = Validators<OrderExt>;
  using InsertValidators = Validators<OrderExt>;
  using CancelValidators = Validators<OrderExt, NoSuchOrderCancelValidator<OrderExt>>;

  PriceTimePriorityMatching<OrderExt, MatchValidators, InsertValidators, CancelValidators>
      matching_engine;

  const auto test_order_id = GenTestOrderID();

  ClientOrderRequest<> test_order_request_cancel{
      OrderSide::BUY,
      OrderAction::CANCEL,
      OrderType::LIMIT,
      test_order_id,
      DEFAULT_TEST_ORDER_SIZE,
      DEFAULT_TEST_ORDER_PRICE,
      DEFAULT_TEST_CLIENT_1_ID,
      DEFAULT_TEST_INSTRUMENT_1_ID
  };

  const ClientOrderRequest<> test_order_request_cancel_clone = test_order_request_cancel;

  PassiveOrderBook<> test_passive_order_book{};
  EngineEventTestObserver test_observer;

  matching_engine.doProcessOrderRequest(test_order_request_cancel, test_passive_order_book, test_observer);

  const auto &order_responses = test_observer.client_order_responses_;
  BOOST_CHECK_EQUAL(order_responses.size(), 1);
  const auto &order_response1 = order_responses[0];
  BOOST_CHECK_EQUAL(order_response1.client_, test_order_request_cancel_clone.client_);
  BOOST_CHECK_EQUAL(order_response1.client_order_id_, test_order_request_cancel_clone.cln_order_id_);
  BOOST_CHECK_EQUAL(order_response1.size_, test_order_request_cancel_clone.size_);
  BOOST_CHECK_EQUAL(order_response1.order_price_, test_order_request_cancel_clone.price_);
  BOOST_CHECK_EQUAL(order_response1.instrument_, test_order_request_cancel_clone.instrument_);
  BOOST_CHECK(order_response1.validation_response_ == ValidationResponse::NO_SUCH_ORDER);
  BOOST_CHECK(order_response1.request_result_ == OrderRequestResult::NACK);
}

} // end of namespace

BOOST_AUTO_TEST_SUITE_END()
