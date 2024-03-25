#pragma once

#include <type_traits>

#include "types.h"
#include "interface/i_validator.hpp"

namespace codetest::matching_engine_sim {

template<typename OrderExt, typename V1 = void, typename... Vs>
struct Validators {
  static ValidationResponse validate(
      const ClientOrderRequest<OrderExt> &order_request,
      const PassiveOrderBook<OrderExt> &passive_order_book,
      const PassiveOrder<OrderExt> &passive_order = PassiveOrder<OrderExt>()) {
    auto validation_response = Validators<OrderExt, V1>::validate(order_request, passive_order_book, passive_order);
    if (validation_response != ValidationResponse::NO_ERROR) return validation_response;
    return Validators<OrderExt, Vs...>::validate(order_request, passive_order_book, passive_order);
  }
};

template<typename OrderExt, typename V>
struct Validators<OrderExt, V> {
  static ValidationResponse validate(
      const ClientOrderRequest<OrderExt> &order_request,
      const PassiveOrderBook<OrderExt> &passive_order_book,
      const PassiveOrder<OrderExt> &passive_order = PassiveOrder<OrderExt>()) {
    if constexpr (std::is_same_v<V, void>) {
      return ValidationResponse::NO_ERROR;
    } else {
      static_assert(std::is_base_of_v<IBaseValidator<OrderExt>, V>, "V must inherit from IBaseValidator");
      return V()(order_request, passive_order_book, passive_order);
    }
  }
};

} // end of namespace