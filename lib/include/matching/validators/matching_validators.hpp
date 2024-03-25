#pragma once

#include "interface/i_validator.hpp"
#include "events/client_order_request.h"
#include "matching/passive_order.h"

namespace codetest::matching_engine_sim {

template<typename OrderExt = void>
struct NoSelfMatchValidator final : public IValidator<NoSelfMatchValidator<OrderExt>, OrderExt> {
  ValidationResponse operator()(
      const ClientOrderRequest<OrderExt> &order_request,
      const PassiveOrderBook<OrderExt> &passive_order_book,
      const PassiveOrder<OrderExt> &passive__order = PassiveOrder<OrderExt>()) override {
    return (order_request.client_ == passive__order.client_) ? ValidationResponse::SELF_MATCH
                                                             : ValidationResponse::NO_ERROR;
  }
};

struct MinExecQtyExtension_MatchValidator final : public IValidator<MinExecQtyExtension_MatchValidator,
                                                                    MinExecQtyExtension> {
  ValidationResponse operator()(
      const ClientOrderRequest<MinExecQtyExtension> &order_request,
      const PassiveOrderBook<MinExecQtyExtension> &passive_order_book,
      const PassiveOrder<MinExecQtyExtension> &passive_order = PassiveOrder<MinExecQtyExtension>()) override {

    return (
               (passive_order.custom_fields_ && order_request.size_ < passive_order.custom_fields_->min_exec_qty_) ||
                   (order_request.custom_fields_
                       && passive_order.remaining_size_ < order_request.custom_fields_->min_exec_qty_))
           ? ValidationResponse::CONTINUE_WITHOUT_MATCHING
           : ValidationResponse::NO_ERROR;
  }
};

} // end of namespace