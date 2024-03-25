#pragma once

#include "types.h"

namespace codetest::matching_engine_sim {

struct ClientOrderResponse {
  constexpr ClientOrderResponse() = default;
  constexpr ClientOrderResponse(const OrderIDType &cln_order_id,
                                const OrderRequestResult &result) : cln_order_id_(cln_order_id), result_(result) {}

  const OrderIDType cln_order_id_{};
  const OrderRequestResult result_{};
};

struct ClientTradeEvent {
  constexpr ClientTradeEvent() = default;
  constexpr ClientTradeEvent(const OrderIDType &cln_order_id,
                             const InstrumentType &instrument,
                             const PriceType &price,
                             const SizeType &size)
      : cln_order_id_(cln_order_id), instrument_(instrument), price_(price), size_(size) {}

  const OrderIDType cln_order_id_{};
  const InstrumentType instrument_{};
  const PriceType price_{};
  const SizeType size_{};
};

} // end of namespace