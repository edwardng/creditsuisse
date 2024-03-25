#pragma once

#include <mutex>
#include <vector>

#include "types.h"
#include "events/client_events.h"
#include "interface/i_engine_event_observer.h"
#include "external/i_client.h"

using namespace codetest::matching_engine_sim;

namespace codetest::matching_engine_sim_test_helper {

struct EngineTradeEventTestRecord {
  EngineTradeEventTestRecord() = default;
  EngineTradeEventTestRecord(
      const ClientType &client1,
      const OrderIDType &client1_order_id,
      const ClientType &client2,
      const OrderIDType &client2_order_id,
      const InstrumentType &instrument,
      const PriceType &trade_price,
      const SizeType &size)
      : client1_(client1),
        client1_order_id_(client1_order_id),
        client2_(client2),
        client2_order_id_(client2_order_id),
        instrument_(instrument),
        trade_price_(trade_price),
        size_(size) {}

  EngineTradeEventTestRecord(const EngineTradeEventTestRecord &) = default;
  EngineTradeEventTestRecord(EngineTradeEventTestRecord &&) noexcept = default;
  EngineTradeEventTestRecord &operator=(const EngineTradeEventTestRecord &) = default;
  EngineTradeEventTestRecord &operator=(EngineTradeEventTestRecord &&) noexcept = default;
  ~EngineTradeEventTestRecord() = default;

  ClientType client1_{};
  OrderIDType client1_order_id_{};
  ClientType client2_{};
  OrderIDType client2_order_id_{};
  InstrumentType instrument_{};
  PriceType trade_price_{};
  SizeType size_{};
};

struct ClientTradeEventTestRecord {
  ClientTradeEventTestRecord() = default;
  ClientTradeEventTestRecord(
      const OrderIDType &client_order_id,
      const InstrumentType &instrument,
      const PriceType &trade_price,
      const SizeType &size)
      : client_order_id_(client_order_id),
        instrument_(instrument),
        trade_price_(trade_price),
        size_(size) {}

  ClientTradeEventTestRecord(const ClientTradeEventTestRecord &) = default;
  ClientTradeEventTestRecord(ClientTradeEventTestRecord &&) noexcept = default;
  ClientTradeEventTestRecord &operator=(const ClientTradeEventTestRecord &) = default;
  ClientTradeEventTestRecord &operator=(ClientTradeEventTestRecord &&) noexcept = default;
  ~ClientTradeEventTestRecord() = default;

  [[nodiscard]] bool operator==(const ClientTradeEventTestRecord &rhs) const;

  OrderIDType client_order_id_{};
  InstrumentType instrument_{};
  PriceType trade_price_{};
  SizeType size_{};
};

struct EngineOrderResponseTestRecord {
  EngineOrderResponseTestRecord() = default;
  EngineOrderResponseTestRecord(
      const ClientType &client,
      const OrderIDType &client_order_id,
      const InstrumentType &instrument,
      const PriceType &order_price,
      const SizeType &size,
      const OrderRequestResult &request_result,
      const ValidationResponse &validation_response)
      : client_(client),
        client_order_id_(client_order_id),
        instrument_(instrument),
        order_price_(order_price),
        size_(size),
        request_result_(request_result),
        validation_response_(validation_response) {}

  EngineOrderResponseTestRecord(const EngineOrderResponseTestRecord &) = default;
  EngineOrderResponseTestRecord(EngineOrderResponseTestRecord &&) noexcept = default;
  EngineOrderResponseTestRecord &operator=(const EngineOrderResponseTestRecord &) = default;
  EngineOrderResponseTestRecord &operator=(EngineOrderResponseTestRecord &&) noexcept = default;
  ~EngineOrderResponseTestRecord() = default;

  [[nodiscard]] bool operator==(const EngineOrderResponseTestRecord &rhs) const;

  ClientType client_{};
  OrderIDType client_order_id_{};
  InstrumentType instrument_{};
  PriceType order_price_{};
  SizeType size_{};
  OrderRequestResult request_result_{};
  ValidationResponse validation_response_{};
};

struct EngineEventTestObserver : IEngineEventObserver {
  void doTradeEvent(
      const ClientType &client1,
      const OrderIDType &client1_order_id,
      const ClientType &client2,
      const OrderIDType &client2_order_id,
      const InstrumentType &instrument,
      const PriceType &trade_price,
      const SizeType &size) override;

  void doOrderRequestResponse(
      const ClientType &client,
      const OrderIDType &client_order_id,
      const InstrumentType &instrument,
      const PriceType &order_price,
      const SizeType &order_size,
      const OrderRequestResult &order_request_result,
      const ValidationResponse &validation_response) override;

  void setClientMap(const std::unordered_map<ClientType, std::shared_ptr<IClient>> &clients) override;

  std::vector<EngineTradeEventTestRecord> client_trade_events_{};
  std::vector<EngineOrderResponseTestRecord> client_order_responses_{};
  std::mutex trade_event_mutex_{};
  std::mutex order_responses_mutex_{};

  std::unordered_map<ClientType, std::shared_ptr<IClient>> clients_{};
};

OrderIDType GenTestOrderID();

// Deliberately using different ranges of ID to help ensure no test coding errors in verifying values between
// ClientID, OrderID, InstrumentID etc
// (if they have same/similar range may unexpectedly match when we do comparison check)
constexpr ClientType DEFAULT_TEST_CLIENT_1_ID{800000001};
constexpr ClientType DEFAULT_TEST_CLIENT_2_ID{800000002};
constexpr ClientType DEFAULT_TEST_CLIENT_3_ID{800000003};
constexpr ClientType DEFAULT_TEST_CLIENT_4_ID{800000004};
constexpr ClientType DEFAULT_TEST_CLIENT_5_ID{800000005};
constexpr ClientType DEFAULT_TEST_CLIENT_6_ID{800000006};

static_assert(DEFAULT_TEST_CLIENT_1_ID != DEFAULT_TEST_CLIENT_2_ID);
static_assert(DEFAULT_TEST_CLIENT_2_ID != DEFAULT_TEST_CLIENT_3_ID);
static_assert(DEFAULT_TEST_CLIENT_3_ID != DEFAULT_TEST_CLIENT_4_ID);
static_assert(DEFAULT_TEST_CLIENT_4_ID != DEFAULT_TEST_CLIENT_5_ID);
static_assert(DEFAULT_TEST_CLIENT_5_ID != DEFAULT_TEST_CLIENT_6_ID);

constexpr SizeType DEFAULT_TEST_ORDER_SIZE{100};
constexpr SizeType DEFAULT_TEST_ORDER_PRICE{100};

constexpr InstrumentType DEFAULT_TEST_INSTRUMENT_1_ID{1001};

struct TestClient : public IClient {
  TestClient() = delete;
  TestClient(const ClientType &client_id) : IClient(client_id) {}
  TestClient(const TestClient &) = delete;
  TestClient(TestClient &&) noexcept = delete;
  TestClient &operator=(TestClient &) = delete;
  TestClient &operator=(TestClient &&) noexcept = delete;
  virtual ~TestClient() = default;

  void onTradeEvent(
      const OrderIDType &client_order_id,
      const InstrumentType &instrument,
      const PriceType &trade_price,
      const SizeType &size);

  void onOrderRequestResponse(
      const OrderIDType &client_order_id,
      const InstrumentType &instrument,
      const PriceType &order_price,
      const SizeType &order_size,
      const OrderRequestResult &order_request_result,
      const ValidationResponse &validation_response);

  [[nodiscard]] const auto &getClientTradeEvents() const { return client_trade_events_; }
  [[nodiscard]] const auto &getOrderResponses() const { return client_order_responses_; }

 private:
  std::vector<ClientTradeEventTestRecord> client_trade_events_{};
  std::vector<EngineOrderResponseTestRecord> client_order_responses_{};
  std::mutex trade_event_mutex_{};
  std::mutex order_responses_mutex_{};
};

} // end of namespace