#include "matching/validators/new_order_request_validators.hpp"

#include <boost/test/unit_test.hpp>

#include "matching/passive_order_book.hpp"
#include "events/client_order_request.h"
#include "test_helper.h"

using namespace codetest::matching_engine_sim;
using namespace codetest::matching_engine_sim_test_helper;

BOOST_AUTO_TEST_SUITE(NewOrderRequestValidatorTest)

namespace codetest::matching_engine_sim_test {

BOOST_AUTO_TEST_CASE(NoSuchOrder) {
  PassiveOrderBook<> test_passive_order_book{};

  ClientOrderRequest<> test_order_request{
      OrderSide::BUY,
      OrderAction::NEW,
      OrderType::LIMIT,
      GenTestOrderID(),
      DEFAULT_TEST_ORDER_SIZE,
      DEFAULT_TEST_ORDER_PRICE,
      DEFAULT_TEST_CLIENT_1_ID,
      DEFAULT_TEST_INSTRUMENT_1_ID
  };

  BOOST_CHECK(
      NoSuchOrderInsertValidator()(test_order_request, test_passive_order_book) == ValidationResponse::NO_ERROR);
}

BOOST_AUTO_TEST_CASE(OrderPreexist) {
  PassiveOrderBook<> test_passive_order_book{};

  ClientOrderRequest<> test_order_request{
      OrderSide::BUY,
      OrderAction::NEW,
      OrderType::LIMIT,
      GenTestOrderID(),
      DEFAULT_TEST_ORDER_SIZE,
      DEFAULT_TEST_ORDER_PRICE,
      DEFAULT_TEST_CLIENT_1_ID,
      DEFAULT_TEST_INSTRUMENT_1_ID
  };

  test_passive_order_book.placePassiveOrder(test_order_request.client_,
                                            test_order_request.cln_order_id_,
                                            test_order_request.order_type_,
                                            test_order_request.side_,
                                            test_order_request.price_,
                                            test_order_request.size_,
                                            test_order_request.custom_fields_);

  BOOST_CHECK(
      NoSuchOrderInsertValidator()(test_order_request, test_passive_order_book)
          == ValidationResponse::ORDER_ID_PREEXIST);
}

BOOST_AUTO_TEST_CASE(OrderSizeWithinLimit) {
  constexpr SizeType MAX_ORDER_SIZE = 1000;

  PassiveOrderBook<> test_passive_order_book{};

  ClientOrderRequest<> test_order_request{
      OrderSide::BUY,
      OrderAction::NEW,
      OrderType::LIMIT,
      GenTestOrderID(),
      DEFAULT_TEST_ORDER_SIZE,
      DEFAULT_TEST_ORDER_PRICE,
      DEFAULT_TEST_CLIENT_1_ID,
      DEFAULT_TEST_INSTRUMENT_1_ID
  };

  BOOST_CHECK(NewOrderRequestSizeValidator<MAX_ORDER_SIZE>()(test_order_request, test_passive_order_book)
                  == ValidationResponse::NO_ERROR);
}

BOOST_AUTO_TEST_CASE(OrderSizeExceedsLimit) {
  constexpr SizeType MAX_ORDER_SIZE = 10;

  PassiveOrderBook<> test_passive_order_book{};

  ClientOrderRequest<> test_order_request{
      OrderSide::BUY,
      OrderAction::NEW,
      OrderType::LIMIT,
      GenTestOrderID(),
      DEFAULT_TEST_ORDER_SIZE,
      DEFAULT_TEST_ORDER_PRICE,
      DEFAULT_TEST_CLIENT_1_ID,
      DEFAULT_TEST_INSTRUMENT_1_ID
  };

  BOOST_CHECK(NewOrderRequestSizeValidator<MAX_ORDER_SIZE>
                  ()(test_order_request, test_passive_order_book) == ValidationResponse::ORDER_SIZE_EXCEED_LIMIT);
}

BOOST_AUTO_TEST_CASE(MinExecQtyExtension_OrderRequestSize_LessThan_MinExecQty) {
  constexpr SizeType ORDER_SIZE = 100;
  constexpr SizeType MIN_EXEC_QTY = 1000;

  PassiveOrderBook<MinExecQtyExtension> test_passive_order_book{};

  ClientOrderRequest<MinExecQtyExtension> test_order_request{
      OrderSide::BUY,
      OrderAction::NEW,
      OrderType::LIMIT,
      GenTestOrderID(),
      ORDER_SIZE,
      DEFAULT_TEST_ORDER_PRICE,
      DEFAULT_TEST_CLIENT_1_ID,
      DEFAULT_TEST_INSTRUMENT_1_ID,
      std::make_shared<MinExecQtyExtension>(MIN_EXEC_QTY)
  };

  BOOST_CHECK(MinExecQtyExtension_InsertValidator()(test_order_request, test_passive_order_book)
                  == ValidationResponse::INVALID_ORDER_REQUEST);
}

BOOST_AUTO_TEST_CASE(MinExecQtyExtension_OrderRequestSize_EqualTo_MinExecQty) {
  constexpr SizeType ORDER_SIZE = 1000;
  constexpr SizeType MIN_EXEC_QTY = 1000;

  PassiveOrderBook<MinExecQtyExtension> test_passive_order_book{};

  ClientOrderRequest<MinExecQtyExtension> test_order_request{
      OrderSide::BUY,
      OrderAction::NEW,
      OrderType::LIMIT,
      GenTestOrderID(),
      ORDER_SIZE,
      DEFAULT_TEST_ORDER_PRICE,
      DEFAULT_TEST_CLIENT_1_ID,
      DEFAULT_TEST_INSTRUMENT_1_ID,
      std::make_shared<MinExecQtyExtension>(MIN_EXEC_QTY)
  };

  BOOST_CHECK(MinExecQtyExtension_InsertValidator()(test_order_request, test_passive_order_book)
                  == ValidationResponse::NO_ERROR);
}

BOOST_AUTO_TEST_CASE(MinExecQtyExtension_OrderRequestSize_GreaterThan_MinExecQty) {
  constexpr SizeType ORDER_SIZE = 1000;
  constexpr SizeType MIN_EXEC_QTY = 10;

  PassiveOrderBook<MinExecQtyExtension> test_passive_order_book{};

  ClientOrderRequest<MinExecQtyExtension> test_order_request{
      OrderSide::BUY,
      OrderAction::NEW,
      OrderType::LIMIT,
      GenTestOrderID(),
      ORDER_SIZE,
      DEFAULT_TEST_ORDER_PRICE,
      DEFAULT_TEST_CLIENT_1_ID,
      DEFAULT_TEST_INSTRUMENT_1_ID,
      std::make_shared<MinExecQtyExtension>(MIN_EXEC_QTY)
  };

  BOOST_CHECK(MinExecQtyExtension_InsertValidator()(test_order_request, test_passive_order_book)
                  == ValidationResponse::NO_ERROR);
}

BOOST_AUTO_TEST_CASE(MinExecQtyExtension_nullptr) {
  constexpr SizeType ORDER_SIZE = 1000;

  PassiveOrderBook<MinExecQtyExtension> test_passive_order_book{};

  ClientOrderRequest<MinExecQtyExtension> test_order_request{
      OrderSide::BUY,
      OrderAction::NEW,
      OrderType::LIMIT,
      GenTestOrderID(),
      ORDER_SIZE,
      DEFAULT_TEST_ORDER_PRICE,
      DEFAULT_TEST_CLIENT_1_ID,
      DEFAULT_TEST_INSTRUMENT_1_ID,
      nullptr
  };

  BOOST_CHECK(MinExecQtyExtension_InsertValidator()(test_order_request, test_passive_order_book)
                  == ValidationResponse::NO_ERROR);
}

} // end of namespace

BOOST_AUTO_TEST_SUITE_END()