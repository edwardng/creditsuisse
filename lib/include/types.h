#pragma once

#include <cstdint>

namespace codetest::matching_engine_sim {

using OrderIDType = std::uint64_t;
using SizeType = std::uint64_t;
using PriceType = std::uint64_t;
using ClientType = std::uint64_t;
using InstrumentType = std::uint64_t;

enum class OrderSide : uint8_t {
  BUY = 0,
  SELL = 1
};

enum class OrderAction : uint8_t {
  NEW = 0,
  CANCEL = 1,
  _ACTION_SIZE_ = 2
};

enum class OrderType : uint8_t {
  LIMIT = 0,
  MARKET = 1
};

enum class OrderRequestResult : uint8_t {
  NACK = 0,
  ACK = 1
};

enum class ValidationResponse : uint8_t {
  NO_ERROR = 0,
  CONTINUE_WITHOUT_MATCHING = 1,
  NO_SUCH_INSTRUMENT = 2,
  NO_SUCH_ORDER = 3,
  ORDER_ID_PREEXIST = 4,
  ORDER_SIZE_EXCEED_LIMIT = 5,
  SELF_MATCH = 6,
  INVALID_ORDER_REQUEST = 7
};

} // end of namespace