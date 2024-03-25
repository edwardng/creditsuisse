#pragma once

#include <unordered_map>
#include <memory>

#include "types.h"
#include "external/i_client.h"

namespace codetest::matching_engine_sim {

struct IEngineEventObserver {
  IEngineEventObserver() = default;
  IEngineEventObserver(const IEngineEventObserver &) = default;
  IEngineEventObserver(IEngineEventObserver &&) noexcept = default;
  virtual IEngineEventObserver &operator=(const IEngineEventObserver &) = default;
  virtual IEngineEventObserver &operator=(IEngineEventObserver &&) noexcept = default;
  virtual ~IEngineEventObserver() = default;

  // from matching perspective there's only one trade event crossed between two counterparties
  virtual void doTradeEvent(
      const ClientType &client1,
      const OrderIDType &client1_order_id,
      const ClientType &client2,
      const OrderIDType &client2_order_id,
      const InstrumentType &instrument,
      const PriceType &trade_price,
      const SizeType &size) = 0;

  virtual void doOrderRequestResponse(
      const ClientType &client,
      const OrderIDType &client_order_id,
      const InstrumentType &instrument,
      const PriceType &order_price,
      const SizeType &order_size,
      const OrderRequestResult &order_request_result,
      const ValidationResponse &validation_response) = 0;

  virtual void setClientMap(const std::unordered_map<ClientType, std::shared_ptr<IClient>> &clients) = 0;
};

} // end of namespace