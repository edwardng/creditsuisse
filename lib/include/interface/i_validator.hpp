#pragma once

#include "types.h"
#include "matching/passive_order.h"

namespace codetest::matching_engine_sim {

template<typename OrderExt>
class ClientOrderRequest;
template<typename OrderExt>
class PassiveOrderBook;
template<typename OrderExt>
class PassiveOrder;

template<typename OrderExt = void>
struct IBaseValidator {
  IBaseValidator() = default;
  IBaseValidator(const IBaseValidator &) = default;
  IBaseValidator(IBaseValidator &&) noexcept = default;
  virtual IBaseValidator &operator=(const IBaseValidator &) = default;
  virtual IBaseValidator &operator=(IBaseValidator &&) noexcept = default;
  virtual ~IBaseValidator() = default;

  virtual ValidationResponse operator()(
      const ClientOrderRequest<OrderExt> &order_request,
      const PassiveOrderBook<OrderExt> &passive_order_book,
      const PassiveOrder<OrderExt> &passive_order = PassiveOrder<OrderExt>()) = 0;
};

template<typename CRTP, typename OrderExt = void>
struct IValidator : public IBaseValidator<OrderExt> {
  ValidationResponse operator()(
      const ClientOrderRequest<OrderExt> &order_request,
      const PassiveOrderBook<OrderExt> &passive_order_book,
      const PassiveOrder<OrderExt> &passive_order = PassiveOrder<OrderExt>()) override {
    return static_cast<CRTP *>(this)->operator()(order_request, passive_order_book, passive_order);
  }
};

} // end of namespace