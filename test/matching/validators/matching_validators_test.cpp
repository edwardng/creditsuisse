#include "matching/validators/matching_validators.hpp"

#include <boost/test/unit_test.hpp>

#include "matching/passive_order_book.hpp"
#include "events/client_order_request.h"
#include "test_helper.h"

using namespace codetest::matching_engine_sim;
using namespace codetest::matching_engine_sim_test_helper;

BOOST_AUTO_TEST_SUITE(MatchingValidatorTest)

namespace codetest::matching_engine_sim_test {

BOOST_AUTO_TEST_CASE(NotSelfMatch) {
  PassiveOrderBook<> test_passive_order_book{};

  const auto test_order_id = GenTestOrderID();

  ClientOrderRequest<> test_order_request{
      OrderSide::BUY,
      OrderAction::NEW,
      OrderType::LIMIT,
      test_order_id,
      DEFAULT_TEST_ORDER_SIZE,
      DEFAULT_TEST_ORDER_PRICE,
      DEFAULT_TEST_CLIENT_1_ID,
      DEFAULT_TEST_INSTRUMENT_1_ID
  };

  PassiveOrder<> test_passive_order{
      DEFAULT_TEST_CLIENT_2_ID,
      test_order_id,
      DEFAULT_TEST_ORDER_SIZE
  };

  BOOST_CHECK(NoSelfMatchValidator()(test_order_request, test_passive_order_book, test_passive_order)
                  == ValidationResponse::NO_ERROR);
}

BOOST_AUTO_TEST_CASE(SelfMatch) {
  PassiveOrderBook<> test_passive_order_book{};

  const auto test_order_id = GenTestOrderID();

  ClientOrderRequest<> test_order_request{
      OrderSide::BUY,
      OrderAction::NEW,
      OrderType::LIMIT,
      test_order_id,
      DEFAULT_TEST_ORDER_SIZE,
      DEFAULT_TEST_ORDER_PRICE,
      DEFAULT_TEST_CLIENT_1_ID,
      DEFAULT_TEST_INSTRUMENT_1_ID
  };

  PassiveOrder<> test_passive_order{
      DEFAULT_TEST_CLIENT_1_ID,
      test_order_id,
      DEFAULT_TEST_ORDER_SIZE
  };

  BOOST_CHECK(NoSelfMatchValidator()(test_order_request, test_passive_order_book, test_passive_order)
                  == ValidationResponse::SELF_MATCH);
}

BOOST_AUTO_TEST_CASE(OrderRequestSize_LessThan_PassiveMinExecQty) {

  constexpr SizeType TEST_ORDER_REQUEST_SIZE = 25;
  constexpr SizeType TEST_ORDER_REQUEST_MIN_EXEC_QTY = 0;

  constexpr SizeType PASSIVE_ORDER_REQUEST_SIZE = 250;
  constexpr SizeType PASSIVE_ORDER_REQUEST_MIN_EXEC_QTY = 150;

  PassiveOrderBook<MinExecQtyExtension> test_passive_order_book;

  const auto test_order_id = GenTestOrderID();

  ClientOrderRequest<MinExecQtyExtension> test_order_request{
      OrderSide::BUY,
      OrderAction::NEW,
      OrderType::LIMIT,
      test_order_id,
      TEST_ORDER_REQUEST_SIZE,
      DEFAULT_TEST_ORDER_PRICE,
      DEFAULT_TEST_CLIENT_1_ID,
      DEFAULT_TEST_INSTRUMENT_1_ID,
      std::make_shared<MinExecQtyExtension>(TEST_ORDER_REQUEST_MIN_EXEC_QTY)
  };

  PassiveOrder<MinExecQtyExtension> test_passive_order{
      DEFAULT_TEST_CLIENT_2_ID,
      test_order_id,
      PASSIVE_ORDER_REQUEST_SIZE,
      std::make_shared<MinExecQtyExtension>(PASSIVE_ORDER_REQUEST_MIN_EXEC_QTY)
  };

  BOOST_CHECK(MinExecQtyExtension_MatchValidator()(test_order_request, test_passive_order_book, test_passive_order)
                  == ValidationResponse::CONTINUE_WITHOUT_MATCHING);

}

BOOST_AUTO_TEST_CASE(PassiveOrderQty_LessThan_OrderRequestMinExecQty) {
  constexpr SizeType TEST_ORDER_REQUEST_SIZE = 250;
  constexpr SizeType TEST_ORDER_REQUEST_MIN_EXEC_QTY = 30;

  constexpr SizeType PASSIVE_ORDER_REQUEST_SIZE = 25;
  constexpr SizeType PASSIVE_ORDER_REQUEST_MIN_EXEC_QTY = 0;

  PassiveOrderBook<MinExecQtyExtension> test_passive_order_book;

  const auto test_order_id = GenTestOrderID();

  ClientOrderRequest<MinExecQtyExtension> test_order_request{
      OrderSide::BUY,
      OrderAction::NEW,
      OrderType::LIMIT,
      test_order_id,
      TEST_ORDER_REQUEST_SIZE,
      DEFAULT_TEST_ORDER_PRICE,
      DEFAULT_TEST_CLIENT_1_ID,
      DEFAULT_TEST_INSTRUMENT_1_ID,
      std::make_shared<MinExecQtyExtension>(TEST_ORDER_REQUEST_MIN_EXEC_QTY)
  };

  PassiveOrder<MinExecQtyExtension> test_passive_order{
      DEFAULT_TEST_CLIENT_2_ID,
      test_order_id,
      PASSIVE_ORDER_REQUEST_SIZE,
      std::make_shared<MinExecQtyExtension>(PASSIVE_ORDER_REQUEST_MIN_EXEC_QTY)
  };

  BOOST_CHECK(MinExecQtyExtension_MatchValidator()(test_order_request, test_passive_order_book, test_passive_order)
                  == ValidationResponse::CONTINUE_WITHOUT_MATCHING);
}

BOOST_AUTO_TEST_CASE(MinExecQty_BothSideZero) {
  constexpr SizeType TEST_ORDER_REQUEST_SIZE = 250;
  constexpr SizeType TEST_ORDER_REQUEST_MIN_EXEC_QTY = 0;

  constexpr SizeType PASSIVE_ORDER_REQUEST_SIZE = 25;
  constexpr SizeType PASSIVE_ORDER_REQUEST_MIN_EXEC_QTY = 0;

  PassiveOrderBook<MinExecQtyExtension> test_passive_order_book;

  const auto test_order_id = GenTestOrderID();

  ClientOrderRequest<MinExecQtyExtension> test_order_request{
      OrderSide::BUY,
      OrderAction::NEW,
      OrderType::LIMIT,
      test_order_id,
      TEST_ORDER_REQUEST_SIZE,
      DEFAULT_TEST_ORDER_PRICE,
      DEFAULT_TEST_CLIENT_1_ID,
      DEFAULT_TEST_INSTRUMENT_1_ID,
      std::make_shared<MinExecQtyExtension>(TEST_ORDER_REQUEST_MIN_EXEC_QTY)
  };

  PassiveOrder<MinExecQtyExtension> test_passive_order{
      DEFAULT_TEST_CLIENT_2_ID,
      test_order_id,
      PASSIVE_ORDER_REQUEST_SIZE,
      std::make_shared<MinExecQtyExtension>(PASSIVE_ORDER_REQUEST_MIN_EXEC_QTY)
  };

  BOOST_CHECK(MinExecQtyExtension_MatchValidator()(test_order_request, test_passive_order_book, test_passive_order)
                  == ValidationResponse::NO_ERROR);
}

BOOST_AUTO_TEST_CASE(Neither_PassiveOrder_OrderRequest_SpecifiedMinExecQty) {
  constexpr SizeType TEST_ORDER_REQUEST_SIZE = 25;
  constexpr SizeType PASSIVE_ORDER_REQUEST_SIZE = 250;

  PassiveOrderBook<MinExecQtyExtension> test_passive_order_book;

  const auto test_order_id = GenTestOrderID();

  ClientOrderRequest<MinExecQtyExtension> test_order_request{
      OrderSide::BUY,
      OrderAction::NEW,
      OrderType::LIMIT,
      test_order_id,
      TEST_ORDER_REQUEST_SIZE,
      DEFAULT_TEST_ORDER_PRICE,
      DEFAULT_TEST_CLIENT_1_ID,
      DEFAULT_TEST_INSTRUMENT_1_ID,
      nullptr // default parameter value is nullptr
  };

  PassiveOrder<MinExecQtyExtension> test_passive_order{
      DEFAULT_TEST_CLIENT_2_ID,
      test_order_id,
      PASSIVE_ORDER_REQUEST_SIZE,
      nullptr // default parameter value is nullptr
  };

  BOOST_CHECK(MinExecQtyExtension_MatchValidator()(test_order_request, test_passive_order_book, test_passive_order)
                  == ValidationResponse::NO_ERROR);
}

BOOST_AUTO_TEST_CASE(PassiveOrder_MinExecQty_nullptr) {

  constexpr SizeType TEST_ORDER_REQUEST_SIZE = 250;
  constexpr SizeType TEST_ORDER_REQUEST_MIN_EXEC_QTY = 30;

  constexpr SizeType PASSIVE_ORDER_REQUEST_SIZE = 50;

  PassiveOrderBook<MinExecQtyExtension> test_passive_order_book;

  const auto test_order_id = GenTestOrderID();

  ClientOrderRequest<MinExecQtyExtension> test_order_request{
      OrderSide::BUY,
      OrderAction::NEW,
      OrderType::LIMIT,
      test_order_id,
      TEST_ORDER_REQUEST_SIZE,
      DEFAULT_TEST_ORDER_PRICE,
      DEFAULT_TEST_CLIENT_1_ID,
      DEFAULT_TEST_INSTRUMENT_1_ID,
      std::make_shared<MinExecQtyExtension>(TEST_ORDER_REQUEST_MIN_EXEC_QTY)
  };

  PassiveOrder<MinExecQtyExtension> test_passive_order{
      DEFAULT_TEST_CLIENT_2_ID,
      test_order_id,
      PASSIVE_ORDER_REQUEST_SIZE,
      nullptr
  };

  BOOST_CHECK(MinExecQtyExtension_MatchValidator()(test_order_request, test_passive_order_book, test_passive_order)
                  == ValidationResponse::NO_ERROR);
}

BOOST_AUTO_TEST_CASE(OrderRequst_MinExecQty_nullptr) {

  constexpr SizeType TEST_ORDER_REQUEST_SIZE = 250;

  constexpr SizeType PASSIVE_ORDER_REQUEST_SIZE = 50;
  constexpr SizeType PASSIVE_ORDER_REQUEST_MIN_EXEC_QTY = 30;

  PassiveOrderBook<MinExecQtyExtension> test_passive_order_book;

  const auto test_order_id = GenTestOrderID();

  ClientOrderRequest<MinExecQtyExtension> test_order_request{
      OrderSide::BUY,
      OrderAction::NEW,
      OrderType::LIMIT,
      test_order_id,
      TEST_ORDER_REQUEST_SIZE,
      DEFAULT_TEST_ORDER_PRICE,
      DEFAULT_TEST_CLIENT_1_ID,
      DEFAULT_TEST_INSTRUMENT_1_ID
  };

  PassiveOrder<MinExecQtyExtension> test_passive_order{
      DEFAULT_TEST_CLIENT_2_ID,
      test_order_id,
      PASSIVE_ORDER_REQUEST_SIZE,
      std::make_shared<MinExecQtyExtension>(PASSIVE_ORDER_REQUEST_MIN_EXEC_QTY)
  };

  BOOST_CHECK(MinExecQtyExtension_MatchValidator()(test_order_request, test_passive_order_book, test_passive_order)
                  == ValidationResponse::NO_ERROR);
}

} // end of namespace

BOOST_AUTO_TEST_SUITE_END()