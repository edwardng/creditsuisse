#include "test_helper.h"

#include <boost/test/unit_test.hpp>

#include "matching/passive_order_book.hpp"

using namespace codetest::matching_engine_sim;
using namespace codetest::matching_engine_sim_test_helper;

BOOST_AUTO_TEST_SUITE(EngineOrderBookTest)

namespace codetest::matching_engine_sim_test {

BOOST_AUTO_TEST_CASE(PlacePassiveBuyOrder) {
  PassiveOrderBook<> test_passive_order_book{};

  const auto test_order_id = GenTestOrderID();

  test_passive_order_book.placePassiveOrder(DEFAULT_TEST_CLIENT_1_ID,
                                            test_order_id,
                                            OrderType::LIMIT,
                                            OrderSide::BUY,
                                            DEFAULT_TEST_ORDER_PRICE,
                                            DEFAULT_TEST_ORDER_SIZE,
                                            nullptr);

  // Expect passive buy on bid queue, empty ask queue
  const auto &askOrderQueue = test_passive_order_book.getAskOrderQueue();
  BOOST_CHECK(askOrderQueue.empty());

  const auto &bidOrderQueue = test_passive_order_book.getBidOrderQueue();
  const auto &[best_bid_price, best_bid_order_queue] {*bidOrderQueue.begin()};

  BOOST_CHECK_EQUAL(best_bid_price, DEFAULT_TEST_ORDER_PRICE);
  BOOST_CHECK_EQUAL(best_bid_order_queue.size(), 1);

  const auto persisted_passive_order = best_bid_order_queue.front();
  BOOST_CHECK_EQUAL(persisted_passive_order->cln_order_id_, test_order_id);
  BOOST_CHECK_EQUAL(persisted_passive_order->client_, DEFAULT_TEST_CLIENT_1_ID);
  BOOST_CHECK_EQUAL(persisted_passive_order->remaining_size_, DEFAULT_TEST_ORDER_SIZE);

  // Expect to find the order in ClientID-OrderID cache
  BOOST_CHECK(test_passive_order_book.isOrderExist(DEFAULT_TEST_CLIENT_1_ID, test_order_id));
  BOOST_CHECK(!test_passive_order_book.isOrderExist(DEFAULT_TEST_CLIENT_2_ID, test_order_id));
  BOOST_CHECK(!test_passive_order_book.isOrderExist(DEFAULT_TEST_CLIENT_1_ID, test_order_id + 1));
  BOOST_CHECK_EQUAL(test_passive_order_book.getEngineOrderFromCache(DEFAULT_TEST_CLIENT_2_ID, test_order_id), nullptr);
  BOOST_CHECK_EQUAL(test_passive_order_book.getEngineOrderFromCache(DEFAULT_TEST_CLIENT_1_ID, test_order_id + 1),
                    nullptr);

  // Expect cached order is the same as the queued order
  const auto
      cached_order_ref = test_passive_order_book.getEngineOrderFromCache(DEFAULT_TEST_CLIENT_1_ID, test_order_id);
  BOOST_CHECK(cached_order_ref == persisted_passive_order);
}

BOOST_AUTO_TEST_CASE(PlacePassiveSellOrder) {
  PassiveOrderBook<> test_passive_order_book{};

  const auto test_order_id = GenTestOrderID();

  test_passive_order_book.placePassiveOrder(DEFAULT_TEST_CLIENT_1_ID,
                                            test_order_id,
                                            OrderType::LIMIT,
                                            OrderSide::SELL,
                                            DEFAULT_TEST_ORDER_PRICE,
                                            DEFAULT_TEST_ORDER_SIZE,
                                            nullptr);

  // Expect passive buy on ask queue, empty bid queue
  const auto &bidOrderQueue = test_passive_order_book.getBidOrderQueue();
  BOOST_CHECK(bidOrderQueue.empty());

  const auto &askOrderQueue = test_passive_order_book.getAskOrderQueue();
  const auto &[best_ask_price, best_ask_order_queue] {*askOrderQueue.begin()};

  BOOST_CHECK_EQUAL(best_ask_price, DEFAULT_TEST_ORDER_PRICE);
  BOOST_CHECK_EQUAL(best_ask_order_queue.size(), 1);

  const auto &persisted_passive_order = best_ask_order_queue.front();
  BOOST_CHECK_EQUAL(persisted_passive_order->cln_order_id_, test_order_id);
  BOOST_CHECK_EQUAL(persisted_passive_order->client_, DEFAULT_TEST_CLIENT_1_ID);
  BOOST_CHECK_EQUAL(persisted_passive_order->remaining_size_, DEFAULT_TEST_ORDER_SIZE);

  // Expect to find the order in ClientID-OrderID cache
  BOOST_CHECK(test_passive_order_book.isOrderExist(DEFAULT_TEST_CLIENT_1_ID, test_order_id));
  BOOST_CHECK(!test_passive_order_book.isOrderExist(DEFAULT_TEST_CLIENT_2_ID, test_order_id));
  BOOST_CHECK(!test_passive_order_book.isOrderExist(DEFAULT_TEST_CLIENT_1_ID, test_order_id + 1));
  BOOST_CHECK_EQUAL(test_passive_order_book.getEngineOrderFromCache(DEFAULT_TEST_CLIENT_2_ID, test_order_id), nullptr);
  BOOST_CHECK_EQUAL(test_passive_order_book.getEngineOrderFromCache(DEFAULT_TEST_CLIENT_1_ID, test_order_id + 1),
                    nullptr);

  // Expect cached order is the same as the queued order
  auto cached_order_ref = test_passive_order_book.getEngineOrderFromCache(DEFAULT_TEST_CLIENT_1_ID, test_order_id);
  BOOST_CHECK(cached_order_ref == persisted_passive_order);
}

BOOST_AUTO_TEST_CASE(RemoveClientOrderCache) {
  PassiveOrderBook<> test_passive_order_book{};

  const auto test_order_id = GenTestOrderID();

  test_passive_order_book.placePassiveOrder(DEFAULT_TEST_CLIENT_1_ID,
                                            test_order_id,
                                            OrderType::LIMIT,
                                            OrderSide::SELL,
                                            DEFAULT_TEST_ORDER_PRICE,
                                            DEFAULT_TEST_ORDER_SIZE,
                                            nullptr);

  // Expect to find the order in ClientID-OrderID cache
  BOOST_CHECK(test_passive_order_book.isOrderExist(DEFAULT_TEST_CLIENT_1_ID, test_order_id));
  test_passive_order_book.cancelClientOrder(DEFAULT_TEST_CLIENT_1_ID, test_order_id);
  BOOST_CHECK(!test_passive_order_book.isOrderExist(DEFAULT_TEST_CLIENT_1_ID, test_order_id));
}

} // end of namespace

BOOST_AUTO_TEST_SUITE_END()