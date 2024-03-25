#include "engine/default_engine_event_handler.h"

namespace codetest::matching_engine_sim {

void DefaultEngineEventHandler::doTradeEvent(
    const ClientType &client1,
    const OrderIDType &client1_order_id,
    const ClientType &client2,
    const OrderIDType &client2_order_id,
    const InstrumentType &instrument,
    const PriceType &trade_price,
    const SizeType &size) {

  auto dispatch_trade = [&, this](const auto &client_id, const auto &client_order_id) {
    if (auto client_itr = clients_.find(client_id); client_itr != clients_.end()) {
      client_itr->second->onTradeEvent(client_order_id, instrument, trade_price, size);
    }
  };

  dispatch_trade(client1, client1_order_id);
  dispatch_trade(client2, client2_order_id);
}

void DefaultEngineEventHandler::doOrderRequestResponse(
    const ClientType &client_id,
    const OrderIDType &client_order_id,
    const InstrumentType &instrument,
    const PriceType &order_price,
    const SizeType &order_size,
    const OrderRequestResult &order_request_result,
    const ValidationResponse &validation_response) {

  if (auto client_itr = clients_.find(client_id); client_itr != clients_.end()) {
    client_itr->second->onOrderRequestResponse(client_order_id,
                                               instrument,
                                               order_price,
                                               order_size,
                                               order_request_result,
                                               validation_response);
  }

}

void DefaultEngineEventHandler::setClientMap(const std::unordered_map<ClientType, std::shared_ptr<IClient>> &clients) {
  clients_ = clients;
}

} // end of namespace