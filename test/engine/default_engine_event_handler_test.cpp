#include <boost/test/unit_test.hpp>

#include "test_helper.h"

#include "engine/default_engine_event_handler.h"

using namespace codetest::matching_engine_sim;
using namespace codetest::matching_engine_sim_test_helper;
using namespace std::literals::chrono_literals;

BOOST_AUTO_TEST_SUITE(DefaultEngineEventHandlerTestSuite)

namespace codetest::matching_engine_sim_test {

BOOST_AUTO_TEST_CASE(DefaultEngineEventHandler_Trades) {

  constexpr ClientType NUMBER_OF_CLIENTS{4};

  std::unordered_map<ClientType, std::shared_ptr<IClient>> clients;
  for (ClientType client_id = 0; client_id < NUMBER_OF_CLIENTS; client_id++) {
    clients[client_id] = std::make_shared<TestClient>(client_id);
  }

  constexpr ClientType CLIENT0_ID{0};
  constexpr OrderIDType CLIENT0_ID_ORDER_ID{1};
  constexpr ClientType CLIENT2_ID{2};
  constexpr OrderIDType CLIENT2_ID_ORDER_ID{11};
  constexpr InstrumentType INSTRUMENT_ID{20};
  constexpr PriceType TRADE_PRICE{100};
  constexpr SizeType TRADE_SIZE{1000};

  DefaultEngineEventHandler handler;
  handler.setClientMap(clients);

  handler.doTradeEvent(CLIENT0_ID,
                       CLIENT0_ID_ORDER_ID,
                       CLIENT2_ID,
                       CLIENT2_ID_ORDER_ID,
                       INSTRUMENT_ID,
                       TRADE_PRICE,
                       TRADE_SIZE);

  const auto &client0 = *std::static_pointer_cast<TestClient>(clients[0]);
  const auto &client1 = *std::static_pointer_cast<TestClient>(clients[1]);
  const auto &client2 = *std::static_pointer_cast<TestClient>(clients[2]);
  const auto &client3 = *std::static_pointer_cast<TestClient>(clients[3]);

  const auto &client0_trades = client0.getClientTradeEvents();
  const auto &client1_trades = client1.getClientTradeEvents();
  const auto &client2_trades = client2.getClientTradeEvents();
  const auto &client3_trades = client3.getClientTradeEvents();

  BOOST_CHECK_EQUAL(client0_trades.size(), 1);
  BOOST_CHECK_EQUAL(client1_trades.size(), 0);
  BOOST_CHECK_EQUAL(client2_trades.size(), 1);
  BOOST_CHECK_EQUAL(client3_trades.size(), 0);

  BOOST_CHECK_EQUAL(client0_trades[0].size_, TRADE_SIZE);
  BOOST_CHECK_EQUAL(client0_trades[0].trade_price_, TRADE_PRICE);
  BOOST_CHECK_EQUAL(client0_trades[0].instrument_, INSTRUMENT_ID);
  BOOST_CHECK_EQUAL(client0_trades[0].client_order_id_, CLIENT0_ID_ORDER_ID);

  BOOST_CHECK_EQUAL(client2_trades[0].size_, TRADE_SIZE);
  BOOST_CHECK_EQUAL(client2_trades[0].trade_price_, TRADE_PRICE);
  BOOST_CHECK_EQUAL(client2_trades[0].instrument_, INSTRUMENT_ID);
  BOOST_CHECK_EQUAL(client2_trades[0].client_order_id_, CLIENT2_ID_ORDER_ID);
}

BOOST_AUTO_TEST_CASE(DefaultEngineEventHandler_OrderResponse) {

  constexpr ClientType NUMBER_OF_CLIENTS{4};

  std::unordered_map<ClientType, std::shared_ptr<IClient>> clients;
  for (ClientType client_id = 0; client_id < NUMBER_OF_CLIENTS; client_id++) {
    clients[client_id] = std::make_shared<TestClient>(client_id);
  }

  constexpr ClientType CLIENT0_ID{0};
  constexpr OrderIDType CLIENT0_ID_ORDER_ID{1};
  constexpr InstrumentType INSTRUMENT_ID{20};
  constexpr PriceType ORDER_PRICE{100};
  constexpr SizeType ORDER_SIZE{1000};
  constexpr OrderRequestResult REQUEST_RESULT{OrderRequestResult::ACK};
  constexpr ValidationResponse VALIDATION_RESPONSE{ValidationResponse::NO_ERROR};

  DefaultEngineEventHandler handler;
  handler.setClientMap(clients);

  handler.doOrderRequestResponse(
      CLIENT0_ID,
      CLIENT0_ID_ORDER_ID,
      INSTRUMENT_ID,
      ORDER_PRICE,
      ORDER_SIZE,
      REQUEST_RESULT,
      VALIDATION_RESPONSE);

  const auto &client0 = *std::static_pointer_cast<TestClient>(clients[0]);
  const auto &client1 = *std::static_pointer_cast<TestClient>(clients[1]);
  const auto &client2 = *std::static_pointer_cast<TestClient>(clients[2]);
  const auto &client3 = *std::static_pointer_cast<TestClient>(clients[3]);

  const auto &client0_order_resp = client0.getOrderResponses();
  const auto &client1_order_resp = client1.getOrderResponses();
  const auto &client2_order_resp = client2.getOrderResponses();
  const auto &client3_order_resp = client3.getOrderResponses();

  BOOST_CHECK_EQUAL(client0_order_resp.size(), 1);
  BOOST_CHECK_EQUAL(client1_order_resp.size(), 0);
  BOOST_CHECK_EQUAL(client2_order_resp.size(), 0);
  BOOST_CHECK_EQUAL(client3_order_resp.size(), 0);

  BOOST_CHECK_EQUAL(client0_order_resp[0].size_, ORDER_SIZE);
  BOOST_CHECK_EQUAL(client0_order_resp[0].order_price_, ORDER_PRICE);
  BOOST_CHECK_EQUAL(client0_order_resp[0].instrument_, INSTRUMENT_ID);
  BOOST_CHECK_EQUAL(client0_order_resp[0].client_order_id_, CLIENT0_ID_ORDER_ID);
  BOOST_CHECK_EQUAL(client0_order_resp[0].client_, CLIENT0_ID);
  BOOST_CHECK(client0_order_resp[0].request_result_ == REQUEST_RESULT);
  BOOST_CHECK(client0_order_resp[0].validation_response_ == VALIDATION_RESPONSE);
}

} // end of namespace

BOOST_AUTO_TEST_SUITE_END()
