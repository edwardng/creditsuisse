#pragma once

#include "interface/i_validator.hpp"
#include "events/client_order_request.h"

namespace codetest::matching_engine_sim {

template<typename OrderExt = void>
struct NoSuchOrderInsertValidator final : public IValidator<NoSuchOrderInsertValidator<OrderExt>, OrderExt> {
  ValidationResponse operator()(
      const ClientOrderRequest<OrderExt> &order_request,
      const PassiveOrderBook<OrderExt> &passive_order_book,
      const PassiveOrder<OrderExt> &passive_order = PassiveOrder<OrderExt>()) override {
    return passive_order_book.isOrderExist(order_request.client_, order_request.cln_order_id_)
           ? ValidationResponse::ORDER_ID_PREEXIST
           : ValidationResponse::NO_ERROR;
  }
};

template<SizeType MAX_ORDER_SIZE, typename OrderExt = void>
struct NewOrderRequestSizeValidator final : public IValidator<NewOrderRequestSizeValidator<MAX_ORDER_SIZE, OrderExt>,
                                                              OrderExt> {
  ValidationResponse operator()(
      const ClientOrderRequest<OrderExt> &order_request,
      const PassiveOrderBook<OrderExt> &passive_order_book,
      const PassiveOrder<OrderExt> &passive_order = PassiveOrder<OrderExt>()) override {
    return (order_request.size_ >= MAX_ORDER_SIZE)
           ? ValidationResponse::ORDER_SIZE_EXCEED_LIMIT
           : ValidationResponse::NO_ERROR;
  }
};

struct MinExecQtyExtension_InsertValidator final : public IValidator<MinExecQtyExtension_InsertValidator,
                                                                     MinExecQtyExtension> {
  ValidationResponse operator()(
      const ClientOrderRequest<MinExecQtyExtension> &order_request,
      const PassiveOrderBook<MinExecQtyExtension> &passive_order_book,
      const PassiveOrder<MinExecQtyExtension> &passive_order = PassiveOrder<MinExecQtyExtension>()) override {

    return (!order_request.custom_fields_
        || (order_request.custom_fields_ && order_request.custom_fields_->min_exec_qty_ <= order_request.size_))
           ? ValidationResponse::NO_ERROR : ValidationResponse::INVALID_ORDER_REQUEST;

  }
};

} // end of namespace