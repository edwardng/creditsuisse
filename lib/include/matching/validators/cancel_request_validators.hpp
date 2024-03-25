#pragma once

#include "interface/i_validator.hpp"
#include "events/client_order_request.h"

namespace codetest::matching_engine_sim {

template<typename OrderExt = void>
struct NoSuchOrderCancelValidator final : public IValidator<NoSuchOrderCancelValidator<OrderExt>, OrderExt> {
  ValidationResponse operator()(
      const ClientOrderRequest<OrderExt> &order_request,
      const PassiveOrderBook<OrderExt> &passive_order_book,
      const PassiveOrder<OrderExt> &passive_order = PassiveOrder<OrderExt>()) override {
    return passive_order_book.isOrderExist(order_request.client_, order_request.cln_order_id_)
           ? ValidationResponse::NO_ERROR
           : ValidationResponse::NO_SUCH_ORDER;
  }
};

} // end of namespace