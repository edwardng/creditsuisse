#pragma once

#include <memory>

#include "types.h"

namespace codetest::matching_engine_sim {

// Not every exchange support minimum quantity per execution
struct MinExecQtyExtension final {
  MinExecQtyExtension() = default;
  MinExecQtyExtension(const SizeType &min_exec_qty)
      : min_exec_qty_(min_exec_qty) {}
  SizeType min_exec_qty_{};
};

template<typename OrderExt = void>
struct alignas(64) ClientOrderRequest final {
  ClientOrderRequest() = default;
  ClientOrderRequest(const OrderSide &side,
                     const OrderAction &action,
                     const OrderType &order_type,
                     const OrderIDType &order_id,
                     const SizeType &size,
                     const PriceType &price,
                     const ClientType &client,
                     const InstrumentType &instrument,
                     const std::shared_ptr<OrderExt> &custom_fields = nullptr) :
      side_(side),
      order_action_(action),
      order_type_(order_type),
      cln_order_id_(order_id),
      size_(size),
      price_(price),
      client_(client),
      instrument_(instrument),
      custom_fields_(custom_fields) {}
  ClientOrderRequest(const ClientOrderRequest &) = default;
  ClientOrderRequest(ClientOrderRequest &&) noexcept = default;
  ClientOrderRequest &operator=(const ClientOrderRequest &) = default;
  ClientOrderRequest &operator=(ClientOrderRequest &&) noexcept = default;
  ~ClientOrderRequest() = default;

  OrderSide side_{};
  OrderAction order_action_{};
  OrderType order_type_{};

  OrderIDType cln_order_id_{};
  SizeType size_{};
  PriceType price_{};
  ClientType client_{};
  InstrumentType instrument_{};
  std::shared_ptr<OrderExt> custom_fields_{nullptr};
};

} // end of namespace