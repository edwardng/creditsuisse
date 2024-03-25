#include <boost/test/unit_test.hpp>

#include <mutex>
#include <vector>
#include <unordered_map>
#include <chrono>

#include "test_helper.h"
#include "engine/matching_engine.h"

using namespace codetest::matching_engine_sim;
using namespace codetest::matching_engine_sim_test_helper;
using namespace std::literals::chrono_literals;

BOOST_AUTO_TEST_SUITE(MatchingEngineTest)

namespace codetest::matching_engine_sim_test {

/*
 * NOTE No efforts invested to ensure changing test setup parameters (various constexpr),
 * that this particular test would continue to work, in particular trades matching.
 * See comment block within the test.
 */

BOOST_AUTO_TEST_CASE(MatchingEngineBasicTest) {

  /**
   * Test Scenario:
   * Matching Engine takes order requests, and match order requests to different instrument order book,
   * in multithreaded processing
   *
   * Test Objectives:
   * 1. Ensure proper routing of all order requests to correct passive order book
   * 2. Ensure order acknowledgements are disseminated
   * 3. Observe correctness via trades generated, no trades are missing, or any unexpected fields
   */

  using OrderExt = void;
  constexpr int NUMBER_OF_INSTRUMENTS = 50;
  constexpr int NUMBER_OF_CLIENTS = 10;
  constexpr std::uint8_t NUMBER_OF_THREADS = 5;

  std::set<InstrumentType> instruments;
  std::unordered_map<ClientType, std::shared_ptr<IClient>> clients;

  std::invoke([&] { // Basic Setup
    for (InstrumentType inst = 0; inst < NUMBER_OF_INSTRUMENTS; inst++) {
      instruments.insert(inst);
    }

    for (ClientType client_id = 0; client_id < NUMBER_OF_CLIENTS; client_id++) {
      clients[client_id] = std::make_shared<TestClient>(client_id);
    }
  });

  auto observer = std::make_shared<EngineEventTestObserver>();
  observer->setClientMap(clients);

  DefaultMatchingEngine<OrderExt> matching_engine{NUMBER_OF_THREADS, instruments, observer};

  constexpr SizeType ORDER_SIZE = 10;
  constexpr SizeType BASE_REFERENCE_PRICE = 10;

  std::vector<EngineOrderResponseTestRecord> expected_outstanding_client_order_responses{};
  std::vector<ClientOrderRequest<OrderExt>> client_order_requests;

  OrderIDType current_order_id{0};

  std::invoke([&] {
    // All limit orders remains not crossed queued as passive, and add to expected order ack list
    // See comments further below

    for (InstrumentType instrument_id = 0; instrument_id < NUMBER_OF_INSTRUMENTS; instrument_id++) {

      PriceType reference_price = BASE_REFERENCE_PRICE * (instrument_id + 1);

      for (ClientType client_id = 0; client_id < NUMBER_OF_CLIENTS / 2; client_id++) {

        client_order_requests.emplace_back(
            OrderSide::BUY,
            OrderAction::NEW,
            OrderType::LIMIT,
            current_order_id,
            ORDER_SIZE,
            static_cast<PriceType>(reference_price + client_id),
            client_id,
            instrument_id);

        expected_outstanding_client_order_responses.emplace_back(
            client_id,
            current_order_id,
            instrument_id,
            static_cast<PriceType>(reference_price + client_id),
            ORDER_SIZE,
            OrderRequestResult::ACK,
            ValidationResponse::NO_ERROR);

        current_order_id++;

        client_order_requests.emplace_back(
            OrderSide::SELL,
            OrderAction::NEW,
            OrderType::LIMIT,
            current_order_id,
            ORDER_SIZE,
            static_cast<PriceType>(reference_price + client_id + (NUMBER_OF_CLIENTS / 2)),
            client_id,
            instrument_id);

        expected_outstanding_client_order_responses.emplace_back(
            client_id,
            current_order_id,
            instrument_id,
            static_cast<PriceType>(reference_price + client_id + (NUMBER_OF_CLIENTS / 2)),
            ORDER_SIZE,
            OrderRequestResult::ACK,
            ValidationResponse::NO_ERROR);

        current_order_id++;
      }
    }
  });

  /*
   * Explanation:
   * For instrument 0, we are building a passive order like this
   *
   * +----------+-------+-----+----------+-------+-----+
   * |         Bid            |         Ask            |
   * +----------+-------+-----+----------+-------+-----+
   * | ClientID | Price | Qty | ClientID | Price | Qty |
   * +----------+-------+-----+----------+-------+-----+
   * | 4        | 14    | 10  | 0        | 15    | 10  |
   * | 3        | 13    | 10  | 1        | 16    | 10  |
   * | 2        | 12    | 10  | 2        | 17    | 10  |
   * | 1        | 11    | 10  | 3        | 18    | 10  |
   * | 0        | 10    | 10  | 4        | 19    | 10  |
   * +----------+-------+-----+----------+-------+-----+
   *
   * Similarly instrument 1, we are building a passive order like below. Each instrument we add 1 to the reference_price
   *
   * +----------+-------+-----+----------+-------+-----+
   * |         Bid            |         Ask            |
   * +----------+-------+-----+----------+-------+-----+
   * | ClientID | Price | Qty | ClientID | Price | Qty |
   * +----------+-------+-----+----------+-------+-----+
   * | 4        | 24    | 10  | 0        | 25    | 10  |
   * | 3        | 23    | 10  | 1        | 26    | 10  |
   * | 2        | 22    | 10  | 2        | 27    | 10  |
   * | 1        | 21    | 10  | 3        | 28    | 10  |
   * | 0        | 20    | 10  | 4        | 29    | 10  |
   * +----------+-------+-----+----------+-------+-----+
   *
   * Variation in prices is intended to make the testing more traceable in case of errors
   *
   * Note the ClientID is shown for illustration purpose
   * In this test artificially made one price level only having one client placing one order
   * We have not send order requests into matching engine but passive order book building
   * is expected to be in above shape, as we expect sequential request processing as per industry standard expectation,
   * which, first come first serves basis must be guaranteed (Price-Time-Priority)
   */

  constexpr SizeType CROSS_SIZE = ORDER_SIZE / 2;
  constexpr PriceType MARKET_ORDER_PRICE = 0;

  std::invoke([&] {
    // Setup market orders, and add to expected order ack list
    // See comments further below

    for (InstrumentType instrument_id = 0; instrument_id < NUMBER_OF_INSTRUMENTS; instrument_id++) {
      for (ClientType client_id = NUMBER_OF_CLIENTS / 2; client_id < NUMBER_OF_CLIENTS; client_id++) {
        client_order_requests.emplace_back(
            OrderSide::BUY,
            OrderAction::NEW,
            OrderType::MARKET,
            current_order_id,
            CROSS_SIZE,
            MARKET_ORDER_PRICE,
            client_id,
            instrument_id);

        expected_outstanding_client_order_responses.emplace_back(
            client_id,
            current_order_id,
            instrument_id,
            MARKET_ORDER_PRICE,
            CROSS_SIZE,
            OrderRequestResult::ACK,
            ValidationResponse::NO_ERROR);

        current_order_id++;

        client_order_requests.emplace_back(
            OrderSide::SELL,
            OrderAction::NEW,
            OrderType::MARKET,
            current_order_id,
            CROSS_SIZE,
            MARKET_ORDER_PRICE,
            client_id,
            instrument_id
        );

        expected_outstanding_client_order_responses.emplace_back(
            client_id,
            current_order_id,
            instrument_id,
            MARKET_ORDER_PRICE,
            CROSS_SIZE,
            OrderRequestResult::ACK,
            ValidationResponse::NO_ERROR);

        current_order_id++;
      }
    }
  });

  /*
   * Post matching market orders we expect the passive order book would look like the following
   * For instrument 0, the order book should be in below shape
   *
   * +----------+-------+-----+----------+-------+-----+
   * |         Bid            |         Ask            |
   * +----------+-------+-----+----------+-------+-----+
   * | ClientID | Price | Qty | ClientID | Price | Qty |
   * +----------+-------+-----+----------+-------+-----+
   * | 2        | 12    | 5   | 2        | 17    | 5   |
   * | 1        | 11    | 10  | 3        | 18    | 10  |
   * | 0        | 10    | 10  | 4        | 19    | 10  |
   * +----------+-------+-----+----------+-------+-----+
   *
   * Similarly for instrument 1
   *
   * +----------+-------+-----+----------+-------+-----+
   * |         Bid            |         Ask            |
   * +----------+-------+-----+----------+-------+-----+
   * | ClientID | Price | Qty | ClientID | Price | Qty |
   * +----------+-------+-----+----------+-------+-----+
   * | 2        | 22    | 5   | 2        | 27    | 5   |
   * | 1        | 21    | 10  | 3        | 28    | 10  |
   * | 0        | 20    | 10  | 4        | 29    | 10  |
   * +----------+-------+-----+----------+-------+-----+
   *
   * We are not intended to verify the residual passive order book as we covered it in other tests
   */

  // Deliberately setting up all order requests in advance,
  // and dispatch all orders to matching engine one after another to maximize parallel processing at the back
  for (const auto &client_order_request : client_order_requests) {
    matching_engine.doOrderRequest(client_order_request);
  }

  std::invoke([&] {
    // Confirm all orders are acknowledged within this time
    constexpr unsigned MAXIMUM_WAITING_TIME_MILLISECOND = 1000;

    std::chrono::time_point<std::chrono::system_clock> start_time = std::chrono::system_clock::now();
    bool all_orders_ack{false};

    while (!all_orders_ack) {
      {
        std::lock_guard<std::mutex> _(observer->order_responses_mutex_);
        all_orders_ack =
            (observer->client_order_responses_.size() == expected_outstanding_client_order_responses.size());
      }

      if ((std::chrono::system_clock::now() - start_time)
          > std::chrono::milliseconds(MAXIMUM_WAITING_TIME_MILLISECOND)) {
        BOOST_FAIL("Not able to complete verifying all expected client order responses within reasonable time");
      }
    }
  });

  std::invoke([&] {
    do {
      bool found{false};

      for (auto expected_response_itr = expected_outstanding_client_order_responses.begin();
           expected_response_itr != expected_outstanding_client_order_responses.end() && !found;
           expected_response_itr++) {

        auto &expected = *expected_response_itr;

        for (auto received_response_itr = observer->client_order_responses_.begin();
             received_response_itr != observer->client_order_responses_.end() && !found;
             received_response_itr++
            ) {
          const auto &current = *received_response_itr;
          if (current == expected) {
            found = true;

            expected_outstanding_client_order_responses.erase(expected_response_itr);
            observer->client_order_responses_.erase(received_response_itr);
          }
        }
      }
    } while (observer->client_order_responses_.size() > 0);

    // At this point all orders acknowledgements are well received

    BOOST_CHECK(observer->client_order_responses_.size() == 0);
  });

  std::vector<EngineTradeEventTestRecord> expected_outstanding_engine_trade_events{};

  constexpr OrderIDType IGNORE_ORDER_ID{0};

  std::invoke([&] {
    // Build expected trades generated
    const auto HALF_NUMBER_OF_CLIENTS = NUMBER_OF_CLIENTS / 2;

    for (InstrumentType instrument_id = 0; instrument_id < NUMBER_OF_INSTRUMENTS; instrument_id++) {
      PriceType reference_price = BASE_REFERENCE_PRICE * (instrument_id + 1);

      ClientType ASK_CLIENT_ID = static_cast<ClientType>(0);
      PriceType ASK_PRICE = static_cast<PriceType>(reference_price + HALF_NUMBER_OF_CLIENTS);
      ClientType BID_CLIENT_ID = static_cast<ClientType>(HALF_NUMBER_OF_CLIENTS - 1);
      PriceType BID_PRICE = static_cast<PriceType>(ASK_PRICE - 1);

      int count{0};

      for (ClientType client_id = HALF_NUMBER_OF_CLIENTS; client_id < NUMBER_OF_CLIENTS; client_id++) {
        expected_outstanding_engine_trade_events.emplace_back(
            client_id,
            IGNORE_ORDER_ID,
            ASK_CLIENT_ID,
            IGNORE_ORDER_ID,
            instrument_id,
            ASK_PRICE,
            static_cast<SizeType>(CROSS_SIZE));

        expected_outstanding_engine_trade_events.emplace_back(
            client_id,
            IGNORE_ORDER_ID,
            BID_CLIENT_ID,
            IGNORE_ORDER_ID,
            instrument_id,
            BID_PRICE,
            static_cast<SizeType>(CROSS_SIZE));

        if (count++ & 1) { // CROSS_SIZE is half of ORDER_SIZE
          ASK_CLIENT_ID++;
          ASK_PRICE++;
          BID_CLIENT_ID--;
          BID_PRICE--;
        }
      }
    }
  });

  std::invoke([&] {
    // Confirm all trades are acknowledged within this time
    constexpr unsigned MAXIMUM_WAITING_TIME_MILLISECOND = 1000;

    std::chrono::time_point<std::chrono::system_clock> start_time = std::chrono::system_clock::now();
    bool all_trades_ack{false};

    while (!all_trades_ack) {
      {
        std::lock_guard<std::mutex> _(observer->trade_event_mutex_);
        all_trades_ack =
            (observer->client_trade_events_.size() == expected_outstanding_engine_trade_events.size());
      }

      if ((std::chrono::system_clock::now() - start_time)
          > std::chrono::milliseconds(MAXIMUM_WAITING_TIME_MILLISECOND)) {
        BOOST_FAIL("Not able to complete verifying all expected client trades events within reasonable time");
      }
    }
  });

  auto compare_engine_trade_test_record =
      [](const EngineTradeEventTestRecord &lhs, const EngineTradeEventTestRecord &rhs) {
        return lhs.instrument_ == rhs.instrument_ &&
            lhs.trade_price_ == rhs.trade_price_ &&
            lhs.size_ == rhs.size_ &&
            lhs.client1_ == rhs.client1_ &&
            lhs.client2_ == rhs.client2_;
      };

  std::invoke([&] {
    do {
      bool found{false};

      for (auto expected_trades_itr = expected_outstanding_engine_trade_events.begin();
           expected_trades_itr != expected_outstanding_engine_trade_events.end() && !found;
           expected_trades_itr++) {

        auto &expected = *expected_trades_itr;

        for (auto received_trades_itr = observer->client_trade_events_.begin();
             received_trades_itr != observer->client_trade_events_.end() && !found;
             received_trades_itr++
            ) {
          const auto &current = *received_trades_itr;
          if (compare_engine_trade_test_record(current, expected)) {
            found = true;

            expected_outstanding_engine_trade_events.erase(expected_trades_itr);
            observer->client_trade_events_.erase(received_trades_itr);
          }
        }
      }
    } while (observer->client_trade_events_.size() > 0);

    // At this point all trades are well received
    BOOST_CHECK(expected_outstanding_engine_trade_events.size() == 0);
    BOOST_CHECK(observer->client_trade_events_.size() == 0);
  });

  matching_engine.terminate();
}

} // end of namespace

BOOST_AUTO_TEST_SUITE_END()
