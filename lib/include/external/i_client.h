#pragma once

#include "types.h"

namespace codetest::matching_engine_sim {

struct IClient {

  IClient() = delete;
  IClient(const ClientType &client_id) : client_id_(client_id) {}
  IClient(const IClient &) = default;
  IClient(IClient &&) noexcept = default;
  IClient &operator=(IClient &) = default;
  IClient &operator=(IClient &&) noexcept = default;
  virtual ~IClient() = default;

  virtual void onTradeEvent(
      const OrderIDType &client_order_id,
      const InstrumentType &instrument,
      const PriceType &trade_price,
      const SizeType &size) = 0;

  virtual void onOrderRequestResponse(
      const OrderIDType &client_order_id,
      const InstrumentType &instrument,
      const PriceType &order_price,
      const SizeType &order_size,
      const OrderRequestResult &order_request_result,
      const ValidationResponse &validation_response) = 0;

  const ClientType getClientID() const { return client_id_; }

 protected:
  ClientType client_id_{};

};

}