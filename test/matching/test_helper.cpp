#include <mutex>

#include "test_helper.h"

namespace codetest::matching_engine_sim_test_helper {

void EngineEventTestObserver::doTradeEvent(
    const ClientType &client1,
    const OrderIDType &client1_order_id,
    const ClientType &client2,
    const OrderIDType &client2_order_id,
    const InstrumentType &instrument,
    const PriceType &trade_price,
    const SizeType &size) {

  {
    std::lock_guard<std::mutex> _{trade_event_mutex_};
    client_trade_events_.emplace_back(
        client1,
        client1_order_id,
        client2,
        client2_order_id,
        instrument,
        trade_price,
        size);
  }

  if (auto itr = clients_.find(client1); itr != clients_.end()) {
    const auto &client = itr->second;
    if (client) client->onTradeEvent(client1_order_id, instrument, trade_price, size);
  }

  if (auto itr = clients_.find(client2); itr != clients_.end()) {
    const auto &client = itr->second;
    if (client) client->onTradeEvent(client2_order_id, instrument, trade_price, size);
  }
}

void EngineEventTestObserver::doOrderRequestResponse(
    const ClientType &client_id,
    const OrderIDType &client_order_id,
    const InstrumentType &instrument,
    const PriceType &order_price,
    const SizeType &order_size,
    const OrderRequestResult &order_request_result,
    const ValidationResponse &validation_response
) {

  {
    std::lock_guard<std::mutex> _{order_responses_mutex_};
    client_order_responses_.emplace_back(
        client_id,
        client_order_id,
        instrument,
        order_price,
        order_size,
        order_request_result,
        validation_response);
  }

  if (auto itr = clients_.find(client_id); itr != clients_.end()) {
    const auto &client = itr->second;
    if (client)
      client->onOrderRequestResponse(client_order_id,
                                     instrument,
                                     order_price,
                                     order_size,
                                     order_request_result,
                                     validation_response);
  }
}

void EngineEventTestObserver::setClientMap(const std::unordered_map<ClientType, std::shared_ptr<IClient>> &clients) {
  clients_ = clients;
}

OrderIDType GenTestOrderID() {
  static std::uint64_t order_id_{0};
  return order_id_++;
}

void TestClient::onTradeEvent(
    const OrderIDType &client_order_id,
    const InstrumentType &instrument,
    const PriceType &trade_price,
    const SizeType &size) {
  std::lock_guard<std::mutex> _{trade_event_mutex_};
  client_trade_events_.emplace_back(client_order_id,
                                    instrument,
                                    trade_price,
                                    size);
}

void TestClient::onOrderRequestResponse(
    const OrderIDType &client_order_id,
    const InstrumentType &instrument,
    const PriceType &order_price,
    const SizeType &order_size,
    const OrderRequestResult &order_request_result,
    const ValidationResponse &validation_response) {
  std::lock_guard<std::mutex> _{order_responses_mutex_};
  client_order_responses_.emplace_back(client_id_,
                                       client_order_id,
                                       instrument,
                                       order_price,
                                       order_size,
                                       order_request_result,
                                       validation_response);
}

[[nodiscard]]
bool EngineOrderResponseTestRecord::operator==(const EngineOrderResponseTestRecord &rhs) const {
  return client_ == rhs.client_ &&
      client_order_id_ == rhs.client_order_id_ &&
      instrument_ == rhs.instrument_ &&
      order_price_ == rhs.order_price_ &&
      size_ == rhs.size_ &&
      request_result_ == rhs.request_result_ &&
      validation_response_ == rhs.validation_response_;
}

[[nodiscard]]
bool ClientTradeEventTestRecord::operator==(const ClientTradeEventTestRecord &rhs) const {
  return client_order_id_ == rhs.client_order_id_ &&
      instrument_ == rhs.instrument_ &&
      trade_price_ == rhs.trade_price_ &&
      size_ == rhs.size_;
}

} // end of namespace