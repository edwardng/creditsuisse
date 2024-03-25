#include "test_helper.h"

#include <boost/test/unit_test.hpp>

#include "matching/validators/new_order_request_validators.hpp"
#include "matching/validators/cancel_request_validators.hpp"
#include "matching/validators/matching_validators.hpp"
#include "matching/matching_algo.hpp"
#include "events/client_order_request.h"

using namespace codetest::matching_engine_sim;
using namespace codetest::matching_engine_sim_test_helper;

BOOST_AUTO_TEST_SUITE(PriceTimePriorityMatching_MinExecQty_TestSuite)

namespace codetest::matching_engine_sim_test {

using MatchValidators = Validators<MinExecQtyExtension,
                                   NoSelfMatchValidator<MinExecQtyExtension>,
                                   MinExecQtyExtension_MatchValidator>;
using InsertValidators = Validators<MinExecQtyExtension,
                                    NoSuchOrderInsertValidator<MinExecQtyExtension>,
                                    MinExecQtyExtension_InsertValidator>;
using CancelValidators = Validators<MinExecQtyExtension,
                                    NoSuchOrderCancelValidator<MinExecQtyExtension>>;

BOOST_AUTO_TEST_CASE(MinExecQty_Matching_InAction)
{
  constexpr SizeType NO_MIN_EXEC_QTY = 0;
  constexpr SizeType MIN_EXEC_QTY = 80;
  constexpr SizeType INCORRECT_MIN_EXEC_QTY = 150;

  constexpr SizeType SELL_ORDER_QTY = 100;
  constexpr SizeType BUY_ORDER_QTY = 150;

  PriceTimePriorityMatching<MinExecQtyExtension, MatchValidators, InsertValidators, CancelValidators>
      matching_engine;

  ClientOrderRequest<MinExecQtyExtension> test_order_request_sell1{
      OrderSide::SELL,
      OrderAction::NEW,
      OrderType::LIMIT,
      GenTestOrderID(),
      SELL_ORDER_QTY,
      DEFAULT_TEST_ORDER_PRICE,
      DEFAULT_TEST_CLIENT_1_ID,
      DEFAULT_TEST_INSTRUMENT_1_ID,
      std::make_shared<MinExecQtyExtension>(NO_MIN_EXEC_QTY)
  };

  ClientOrderRequest<MinExecQtyExtension> test_order_request_sell2{
      OrderSide::SELL,
      OrderAction::NEW,
      OrderType::LIMIT,
      GenTestOrderID(),
      SELL_ORDER_QTY,
      DEFAULT_TEST_ORDER_PRICE,
      DEFAULT_TEST_CLIENT_2_ID,
      DEFAULT_TEST_INSTRUMENT_1_ID,
      std::make_shared<MinExecQtyExtension>(INCORRECT_MIN_EXEC_QTY)
  };

  ClientOrderRequest<MinExecQtyExtension> test_order_request_sell3{
      OrderSide::SELL,
      OrderAction::NEW,
      OrderType::LIMIT,
      GenTestOrderID(),
      SELL_ORDER_QTY,
      DEFAULT_TEST_ORDER_PRICE,
      DEFAULT_TEST_CLIENT_3_ID,
      DEFAULT_TEST_INSTRUMENT_1_ID,
      std::make_shared<MinExecQtyExtension>(MIN_EXEC_QTY)
  };

  ClientOrderRequest<MinExecQtyExtension> test_order_request_sell4{
      OrderSide::SELL,
      OrderAction::NEW,
      OrderType::LIMIT,
      GenTestOrderID(),
      SELL_ORDER_QTY,
      DEFAULT_TEST_ORDER_PRICE,
      DEFAULT_TEST_CLIENT_4_ID,
      DEFAULT_TEST_INSTRUMENT_1_ID,
      std::make_shared<MinExecQtyExtension>(NO_MIN_EXEC_QTY)
  };

  ClientOrderRequest<MinExecQtyExtension> test_order_request_buy{
      OrderSide::BUY,
      OrderAction::NEW,
      OrderType::MARKET,
      GenTestOrderID(),
      BUY_ORDER_QTY,
      DEFAULT_TEST_ORDER_PRICE,
      DEFAULT_TEST_CLIENT_5_ID,
      DEFAULT_TEST_INSTRUMENT_1_ID,
      std::make_shared<MinExecQtyExtension>(NO_MIN_EXEC_QTY)
  };

  const ClientOrderRequest<MinExecQtyExtension> test_order_request_sell1_clone = test_order_request_sell1;
  const ClientOrderRequest<MinExecQtyExtension> test_order_request_sell2_clone = test_order_request_sell2;
  const ClientOrderRequest<MinExecQtyExtension> test_order_request_sell3_clone = test_order_request_sell3;
  const ClientOrderRequest<MinExecQtyExtension> test_order_request_sell4_clone = test_order_request_sell4;
  const ClientOrderRequest<MinExecQtyExtension> test_order_request_buy_clone = test_order_request_buy;

  PassiveOrderBook<MinExecQtyExtension> test_passive_order_book{};
  EngineEventTestObserver test_observer;

  matching_engine.doProcessOrderRequest(test_order_request_sell1, test_passive_order_book, test_observer);
  matching_engine.doProcessOrderRequest(test_order_request_sell2, test_passive_order_book, test_observer);
  matching_engine.doProcessOrderRequest(test_order_request_sell3, test_passive_order_book, test_observer);
  matching_engine.doProcessOrderRequest(test_order_request_sell4, test_passive_order_book, test_observer);
  matching_engine.doProcessOrderRequest(test_order_request_buy, test_passive_order_book, test_observer);

  // Expectations:
  // sell1 fully executed
  // sell2 rejected (MinExecQty > OrderRequest Size) so no cross
  // sell3 ignored in this buy matching (MinExecQty 80 > Buy Order Remaining Qty 50)
  // sell4 executed 50
  // buy order fully executed

  const auto &ask_order_queue{test_passive_order_book.getAskOrderQueue()};
  BOOST_CHECK_EQUAL(ask_order_queue.size(), 1);

  const auto &bid_order_queue{test_passive_order_book.getBidOrderQueue()};
  BOOST_CHECK_EQUAL(bid_order_queue.size(), 0);

  const auto &[best_ask_price, best_ask_order_queue] {*ask_order_queue.begin()};
  BOOST_CHECK_EQUAL(best_ask_price, test_order_request_sell1_clone.price_);
  BOOST_CHECK_EQUAL(best_ask_order_queue.size(), 2);

  const auto &first_best_ask_order = best_ask_order_queue.front();
  BOOST_CHECK_EQUAL(first_best_ask_order->remaining_size_, test_order_request_sell3_clone.size_);
  BOOST_CHECK_EQUAL(first_best_ask_order->cln_order_id_, test_order_request_sell3_clone.cln_order_id_);

  const SizeType REMAINING_QTY = test_order_request_buy_clone.size_ - test_order_request_sell1_clone.size_;

  const auto &last_best_ask_order = best_ask_order_queue.back();
  BOOST_CHECK_EQUAL(last_best_ask_order->remaining_size_, REMAINING_QTY);
  BOOST_CHECK_EQUAL(last_best_ask_order->cln_order_id_, test_order_request_sell4_clone.cln_order_id_);

  // Expect 2 trades
  BOOST_CHECK(test_observer.client_trade_events_.size() == 2);
  const auto &client_trade1 = test_observer.client_trade_events_[0];
  BOOST_CHECK_EQUAL(client_trade1.trade_price_, test_order_request_sell1_clone.price_);
  BOOST_CHECK_EQUAL(client_trade1.size_, test_order_request_sell1_clone.size_);
  BOOST_CHECK_EQUAL(client_trade1.client1_, test_order_request_buy_clone.client_);
  BOOST_CHECK_EQUAL(client_trade1.client1_order_id_, test_order_request_buy_clone.cln_order_id_);
  BOOST_CHECK_EQUAL(client_trade1.client2_, test_order_request_sell1_clone.client_);
  BOOST_CHECK_EQUAL(client_trade1.client2_order_id_, test_order_request_sell1_clone.cln_order_id_);

  const auto &client_trade2 = test_observer.client_trade_events_[1];
  BOOST_CHECK_EQUAL(client_trade2.trade_price_, test_order_request_sell4_clone.price_);
  BOOST_CHECK_EQUAL(client_trade2.size_, REMAINING_QTY);
  BOOST_CHECK_EQUAL(client_trade2.client1_, test_order_request_buy_clone.client_);
  BOOST_CHECK_EQUAL(client_trade2.client1_order_id_, test_order_request_buy_clone.cln_order_id_);
  BOOST_CHECK_EQUAL(client_trade2.client2_, test_order_request_sell4_clone.client_);
  BOOST_CHECK_EQUAL(client_trade2.client2_order_id_, test_order_request_sell4_clone.cln_order_id_);
}

} // end of namespace

BOOST_AUTO_TEST_SUITE_END()
