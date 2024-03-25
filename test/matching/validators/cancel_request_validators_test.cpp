#include "matching/validators/cancel_request_validators.hpp"

#include <boost/test/unit_test.hpp>

#include "matching/passive_order_book.hpp"
#include "events/client_order_request.h"
#include "test_helper.h"

using namespace codetest::matching_engine_sim;
using namespace codetest::matching_engine_sim_test_helper;

BOOST_AUTO_TEST_SUITE(CancelRequestValidatorTest)

namespace codetest::matching_engine_sim_test {

BOOST_AUTO_TEST_CASE(NoSuchOrderBeforeCancel) {
  PassiveOrderBook<> test_passive_order_book{};

  ClientOrderRequest<> test_order_request{
      OrderSide::BUY,
      OrderAction::CANCEL,
      OrderType::LIMIT,
      GenTestOrderID(),
      DEFAULT_TEST_ORDER_SIZE,
      DEFAULT_TEST_ORDER_PRICE,
      DEFAULT_TEST_CLIENT_1_ID,
      DEFAULT_TEST_INSTRUMENT_1_ID
  };

  BOOST_CHECK(
      NoSuchOrderCancelValidator()(test_order_request, test_passive_order_book) == ValidationResponse::NO_SUCH_ORDER);
}

BOOST_AUTO_TEST_CASE(OrderPreexistBeforeCancel) {
  PassiveOrderBook<> test_passive_order_book{};

  const OrderIDType TEST_ORDER_ID_1 = GenTestOrderID();

  ClientOrderRequest<> test_order_request{
      OrderSide::BUY,
      OrderAction::NEW,
      OrderType::LIMIT,
      TEST_ORDER_ID_1,
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

  ClientOrderRequest<> test_order_request_cancel{
      OrderSide::BUY,
      OrderAction::CANCEL,
      OrderType::LIMIT,
      TEST_ORDER_ID_1,
      DEFAULT_TEST_ORDER_SIZE,
      DEFAULT_TEST_ORDER_PRICE,
      DEFAULT_TEST_CLIENT_1_ID,
      DEFAULT_TEST_INSTRUMENT_1_ID
  };

  BOOST_CHECK(
      NoSuchOrderCancelValidator()(test_order_request_cancel, test_passive_order_book)
          == ValidationResponse::NO_ERROR);
}

} // end of namespace

BOOST_AUTO_TEST_SUITE_END()
