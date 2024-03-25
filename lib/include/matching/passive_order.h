#pragma once

#include <memory>

namespace codetest::matching_engine_sim {

template<typename OrderExt = void>
struct PassiveOrder {
  constexpr PassiveOrder() = default;

  constexpr PassiveOrder(const ClientType &client,
                         const OrderIDType &cln_order_id,
                         const SizeType &remaining_size,
                         const std::shared_ptr<OrderExt> &custom_fields = nullptr)
      : client_(client),
        cln_order_id_(cln_order_id),
        remaining_size_(remaining_size),
        custom_fields_(custom_fields) {}

  const ClientType client_{};
  const OrderIDType cln_order_id_{};
  SizeType remaining_size_{};
  std::shared_ptr<OrderExt> custom_fields_{};
};

} // end of namespace