#pragma once

#include "interface/i_engine_event_observer.h"

namespace codetest::matching_engine_sim {

struct DefaultEngineEventHandler : public IEngineEventObserver {

  DefaultEngineEventHandler() = default;
  DefaultEngineEventHandler(const DefaultEngineEventHandler &) = default;
  DefaultEngineEventHandler(DefaultEngineEventHandler &&) noexcept = default;
  virtual DefaultEngineEventHandler &operator=(const DefaultEngineEventHandler &) = default;
  virtual DefaultEngineEventHandler &operator=(DefaultEngineEventHandler &&) noexcept = default;
  virtual ~DefaultEngineEventHandler() = default;

  void doTradeEvent(
      const ClientType &client1,
      const OrderIDType &client1_order_id,
      const ClientType &client2,
      const OrderIDType &client2_order_id,
      const InstrumentType &instrument,
      const PriceType &trade_price,
      const SizeType &size) override;

  void doOrderRequestResponse(
      const ClientType &client_id,
      const OrderIDType &client_order_id,
      const InstrumentType &instrument,
      const PriceType &order_price,
      const SizeType &order_size,
      const OrderRequestResult &order_request_result,
      const ValidationResponse &validation_response) override;

  void setClientMap(const std::unordered_map<ClientType, std::shared_ptr<IClient>> &clients) override;
  [[nodiscard]] const auto &getClientsMap() const { return clients_; }

 protected:
  std::unordered_map<ClientType, std::shared_ptr<IClient>> clients_{};

};

} // end of namespace