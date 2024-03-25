#include "test_helper.h"

#include <boost/test/unit_test.hpp>

#include "matching/matching_algo.hpp"
#include "events/client_order_request.h"

using namespace codetest::matching_engine_sim;
using namespace codetest::matching_engine_sim_test_helper;

BOOST_AUTO_TEST_SUITE(PriceTimePriorityMatching_Validators_TestSuite)

namespace codetest::matching_engine_sim_test {

BOOST_AUTO_TEST_CASE(New_LimitBuyOrder_ClientOrderIDRepeated)
{
  using NewValidators = Validators<NoOrderExt, NoSuchOrderInsertValidator<NoOrderExt>>;
  using MatchValidators = Validators<NoOrderExt>;
  PriceTimePriorityMatching<NoOrderExt, MatchValidators, NewValidators> matching_engine;

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

  auto verify_states = [](
      const ClientOrderRequest<> &test_order_request_clone,
      const PassiveOrderBook<> &test_passive_order_book,
      const EngineEventTestObserver &test_observer) {
    // Expect the buy order placed on bid queue, and empty ask queue
    const auto &ask_order_queue{test_passive_order_book.getAskOrderQueue()};
    BOOST_CHECK_EQUAL(ask_order_queue.size(), 0);

    const auto &bid_order_queue{test_passive_order_book.getBidOrderQueue()};
    BOOST_CHECK_EQUAL(bid_order_queue.size(), 1);

    auto first_bid_level_itr{bid_order_queue.begin()};
    BOOST_CHECK(first_bid_level_itr != bid_order_queue.end());

    const auto &[best_bid_price, best_bid_order_queue] {*first_bid_level_itr};

    BOOST_CHECK_EQUAL(best_bid_price, test_order_request_clone.price_);
    BOOST_CHECK_EQUAL(best_bid_order_queue.size(), 1);

    const auto &persisted_passive_order{best_bid_order_queue.front()};
    BOOST_CHECK_EQUAL(persisted_passive_order->cln_order_id_, test_order_request_clone.cln_order_id_);
    BOOST_CHECK_EQUAL(persisted_passive_order->client_, test_order_request_clone.client_);
    BOOST_CHECK_EQUAL(persisted_passive_order->remaining_size_, test_order_request_clone.size_);

    // Expect no trades reported
    BOOST_CHECK(test_observer.client_trade_events_.size() == 0);
  };

  matching_engine.doProcessOrderRequest(test_order_request, test_passive_order_book, test_observer);
  verify_states(test_order_request_clone, test_passive_order_book, test_observer);

  // Not changing the order queue at all with repeated ClientID-OrderID pre-existing
  matching_engine.doProcessOrderRequest(test_order_request, test_passive_order_book, test_observer);
  verify_states(test_order_request_clone, test_passive_order_book, test_observer);
}

BOOST_AUTO_TEST_CASE(New_LimitBuyOrder_ExceedsOrderSizeLimit)
{
  constexpr SizeType MAX_ORDER_SIZE = 50;
  constexpr SizeType ORDER_1_SIZE = 1;
  constexpr SizeType ORDER_2_SIZE = 100;

  using NewValidators = Validators<NoOrderExt,
                                   NoSuchOrderInsertValidator<NoOrderExt>,
                                   NewOrderRequestSizeValidator<MAX_ORDER_SIZE>>;
  using MatchValidators = Validators<NoOrderExt>;
  PriceTimePriorityMatching<NoOrderExt, MatchValidators, NewValidators> matching_engine;

  ClientOrderRequest<> test_order_request_buy1{
      OrderSide::BUY,
      OrderAction::NEW,
      OrderType::LIMIT,
      GenTestOrderID(),
      ORDER_1_SIZE,
      DEFAULT_TEST_ORDER_PRICE,
      DEFAULT_TEST_CLIENT_1_ID,
      DEFAULT_TEST_INSTRUMENT_1_ID
  };

  ClientOrderRequest<> test_order_request_buy2{
      OrderSide::BUY,
      OrderAction::NEW,
      OrderType::LIMIT,
      GenTestOrderID(),
      ORDER_2_SIZE,
      DEFAULT_TEST_ORDER_PRICE,
      DEFAULT_TEST_CLIENT_1_ID,
      DEFAULT_TEST_INSTRUMENT_1_ID
  };

  const ClientOrderRequest<> test_order_request_buy1_clone = test_order_request_buy1;
  const ClientOrderRequest<> test_order_request_buy2_clone = test_order_request_buy2;

  PassiveOrderBook<> test_passive_order_book{};
  EngineEventTestObserver test_observer;

  auto verify_states = [](
      const ClientOrderRequest<> &test_order_request_clone,
      const PassiveOrderBook<> &test_passive_order_book,
      const EngineEventTestObserver &test_observer) {
    // Expect the buy order placed on bid queue, and empty ask queue
    const auto &ask_order_queue{test_passive_order_book.getAskOrderQueue()};
    BOOST_CHECK_EQUAL(ask_order_queue.size(), 0);

    const auto &bid_order_queue{test_passive_order_book.getBidOrderQueue()};
    BOOST_CHECK_EQUAL(bid_order_queue.size(), 1);

    auto first_bid_level_itr = bid_order_queue.begin();
    BOOST_CHECK(first_bid_level_itr != bid_order_queue.end());

    const auto &[best_bid_price, best_bid_order_queue] = *first_bid_level_itr;

    BOOST_CHECK_EQUAL(best_bid_price, test_order_request_clone.price_);
    BOOST_CHECK_EQUAL(best_bid_order_queue.size(), 1);

    const auto &persisted_passive_order = best_bid_order_queue.front();
    BOOST_CHECK_EQUAL(persisted_passive_order->cln_order_id_, test_order_request_clone.cln_order_id_);
    BOOST_CHECK_EQUAL(persisted_passive_order->client_, test_order_request_clone.client_);
    BOOST_CHECK_EQUAL(persisted_passive_order->remaining_size_, test_order_request_clone.size_);

    // Expect no trades reported
    BOOST_CHECK(test_observer.client_trade_events_.size() == 0);
  };

  // Expects only first order (buy1) is placed to bid order queue as the second one is rejected
  matching_engine.doProcessOrderRequest(test_order_request_buy1, test_passive_order_book, test_observer);
  verify_states(test_order_request_buy1_clone, test_passive_order_book, test_observer);

  // Not changing the order queue at all with repeated ClientID-OrderID pre-existing
  matching_engine.doProcessOrderRequest(test_order_request_buy2, test_passive_order_book, test_observer);
  verify_states(test_order_request_buy1_clone, test_passive_order_book, test_observer);
}

} // end of namespace

BOOST_AUTO_TEST_SUITE_END()
