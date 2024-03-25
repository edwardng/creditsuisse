#include "matching/validators/validators.hpp"

#include <boost/test/unit_test.hpp>

#include "matching/validators/new_order_request_validators.hpp"

#include "events/client_order_request.h"
#include "matching/passive_order_book.hpp"

#include "test_helper.h"

using namespace codetest::matching_engine_sim;
using namespace codetest::matching_engine_sim_test_helper;

BOOST_AUTO_TEST_SUITE(ValidatorsTest)

namespace codetest::matching_engine_sim_test {

BOOST_AUTO_TEST_CASE(Validators_OrderPreexist_WithinOrderSizeLimit) {
  constexpr SizeType MAX_ORDER_SIZE = DEFAULT_TEST_ORDER_SIZE + 1;

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
                                            nullptr);

  auto validation_response =
      Validators<void, NoSuchOrderInsertValidator<>, NewOrderRequestSizeValidator<MAX_ORDER_SIZE>>::validate
          (test_order_request, test_passive_order_book);

  BOOST_CHECK(validation_response == ValidationResponse::ORDER_ID_PREEXIST);
}

BOOST_AUTO_TEST_CASE(Validators_OrderPreexist_ExceedsOrderSizeLimit) {
  constexpr SizeType MAX_ORDER_SIZE = DEFAULT_TEST_ORDER_SIZE - 1;

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

  auto validation_response =
      Validators<void, NoSuchOrderInsertValidator<>, NewOrderRequestSizeValidator<MAX_ORDER_SIZE>>::validate
          (test_order_request, test_passive_order_book);

  BOOST_CHECK(validation_response == ValidationResponse::ORDER_SIZE_EXCEED_LIMIT);
}

BOOST_AUTO_TEST_CASE(Validators_OrderNotExist_WithinOrderSizeLimit) {
  constexpr SizeType MAX_ORDER_SIZE = DEFAULT_TEST_ORDER_SIZE + 1;

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

  auto validation_response =
      Validators<void, NoSuchOrderInsertValidator<>, NewOrderRequestSizeValidator<MAX_ORDER_SIZE>>::validate
          (test_order_request, test_passive_order_book);

  BOOST_CHECK(validation_response == ValidationResponse::NO_ERROR);
}

BOOST_AUTO_TEST_CASE(Validators_OrderNotExist_ExceedsOrderSizeLimit) {
  constexpr SizeType MAX_ORDER_SIZE = DEFAULT_TEST_ORDER_SIZE - 1;

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

  auto validation_response =
      Validators<void, NoSuchOrderInsertValidator<>, NewOrderRequestSizeValidator<MAX_ORDER_SIZE>>::validate
          (test_order_request, test_passive_order_book);

  BOOST_CHECK(validation_response == ValidationResponse::ORDER_SIZE_EXCEED_LIMIT);
}

} // end of namespace

BOOST_AUTO_TEST_SUITE_END()