#include "test_helper.h"

#include <boost/test/unit_test.hpp>

#include "matching/validators/matching_validators.hpp"
#include "matching/matching_algo.hpp"
#include "events/client_order_request.h"

using namespace codetest::matching_engine_sim;
using namespace codetest::matching_engine_sim_test_helper;

BOOST_AUTO_TEST_SUITE(PriceTimePriorityMatching_OrderResponse_TestSuite)

namespace codetest::matching_engine_sim_test {

BOOST_AUTO_TEST_CASE(InsertAck)
{
  PriceTimePriorityMatching<> matching_engine;

  ClientOrderRequest<> test_order_request{
      OrderSide::SELL,
      OrderAction::NEW,
      OrderType::LIMIT,
      GenTestOrderID(),
      DEFAULT_TEST_ORDER_SIZE,
      DEFAULT_TEST_ORDER_PRICE,
      DEFAULT_TEST_CLIENT_1_ID,
      DEFAULT_TEST_INSTRUMENT_1_ID
  };

  const ClientOrderRequest<> test_order_request_clone = test_order_request;

  PassiveOrderBook<> test_passive_order_book{};
  EngineEventTestObserver test_observer;

  matching_engine.doProcessOrderRequest(test_order_request, test_passive_order_book, test_observer);

  BOOST_CHECK(test_observer.client_order_responses_.size() == 1);
  const auto &client_order_response = test_observer.client_order_responses_[0];
  BOOST_CHECK_EQUAL(client_order_response.client_, test_order_request_clone.client_);
  BOOST_CHECK_EQUAL(client_order_response.client_order_id_, test_order_request_clone.cln_order_id_);
  BOOST_CHECK_EQUAL(client_order_response.instrument_, test_order_request_clone.instrument_);
  BOOST_CHECK_EQUAL(client_order_response.order_price_, test_order_request_clone.price_);
  BOOST_CHECK_EQUAL(client_order_response.size_, test_order_request_clone.size_);
  BOOST_CHECK(client_order_response.request_result_ == OrderRequestResult::ACK);
  BOOST_CHECK(client_order_response.validation_response_ == ValidationResponse::NO_ERROR);
}

BOOST_AUTO_TEST_CASE(DuplicatedInsertNack)
{
  using NewValidators = Validators<NoOrderExt, NoSuchOrderInsertValidator<>>;
  using MatchValidators = Validators<NoOrderExt>;
  PriceTimePriorityMatching<NoOrderExt, MatchValidators, NewValidators> matching_engine;

  ClientOrderRequest<> test_order_request{
      OrderSide::SELL,
      OrderAction::NEW,
      OrderType::LIMIT,
      GenTestOrderID(),
      DEFAULT_TEST_ORDER_SIZE,
      DEFAULT_TEST_ORDER_PRICE,
      DEFAULT_TEST_CLIENT_1_ID,
      DEFAULT_TEST_INSTRUMENT_1_ID
  };

  const ClientOrderRequest<> test_order_request_clone = test_order_request;

  PassiveOrderBook<> test_passive_order_book{};
  EngineEventTestObserver test_observer;

  matching_engine.doProcessOrderRequest(test_order_request, test_passive_order_book, test_observer);
  matching_engine.doProcessOrderRequest(test_order_request, test_passive_order_book, test_observer);

  BOOST_CHECK(test_observer.client_order_responses_.size() == 2);

  const auto &client_order_response1 = test_observer.client_order_responses_[0];
  BOOST_CHECK_EQUAL(client_order_response1.client_, test_order_request_clone.client_);
  BOOST_CHECK_EQUAL(client_order_response1.client_order_id_, test_order_request_clone.cln_order_id_);
  BOOST_CHECK_EQUAL(client_order_response1.instrument_, test_order_request_clone.instrument_);
  BOOST_CHECK_EQUAL(client_order_response1.order_price_, test_order_request_clone.price_);
  BOOST_CHECK_EQUAL(client_order_response1.size_, test_order_request_clone.size_);
  BOOST_CHECK(client_order_response1.request_result_ == OrderRequestResult::ACK);
  BOOST_CHECK(client_order_response1.validation_response_ == ValidationResponse::NO_ERROR);

  const auto &client_order_response2 = test_observer.client_order_responses_[1];
  BOOST_CHECK_EQUAL(client_order_response2.client_, test_order_request_clone.client_);
  BOOST_CHECK_EQUAL(client_order_response2.client_order_id_, test_order_request_clone.cln_order_id_);
  BOOST_CHECK_EQUAL(client_order_response2.instrument_, test_order_request_clone.instrument_);
  BOOST_CHECK_EQUAL(client_order_response2.order_price_, test_order_request_clone.price_);
  BOOST_CHECK_EQUAL(client_order_response2.size_, test_order_request_clone.size_);
  BOOST_CHECK(client_order_response2.request_result_ == OrderRequestResult::NACK);
  BOOST_CHECK(client_order_response2.validation_response_ == ValidationResponse::ORDER_ID_PREEXIST);
}

BOOST_AUTO_TEST_CASE(SelfMatch)
{
  using NewValidators = Validators<NoOrderExt, NoSuchOrderInsertValidator<>>;
  using MatchValidators = Validators<NoOrderExt, NoSelfMatchValidator<>>;
  PriceTimePriorityMatching<NoOrderExt, MatchValidators, NewValidators> matching_engine;

  ClientOrderRequest<> test_order_request_sell{
      OrderSide::SELL,
      OrderAction::NEW,
      OrderType::LIMIT,
      GenTestOrderID(),
      DEFAULT_TEST_ORDER_SIZE,
      DEFAULT_TEST_ORDER_PRICE,
      DEFAULT_TEST_CLIENT_1_ID,
      DEFAULT_TEST_INSTRUMENT_1_ID
  };

  ClientOrderRequest<> test_order_request_buy{
      OrderSide::BUY,
      OrderAction::NEW,
      OrderType::LIMIT,
      GenTestOrderID(),
      DEFAULT_TEST_ORDER_SIZE,
      DEFAULT_TEST_ORDER_PRICE,
      DEFAULT_TEST_CLIENT_1_ID,
      DEFAULT_TEST_INSTRUMENT_1_ID
  };

  const ClientOrderRequest<> test_order_request_sell_clone = test_order_request_sell;
  const ClientOrderRequest<> test_order_request_buy_clone = test_order_request_buy;

  PassiveOrderBook<> test_passive_order_book{};
  EngineEventTestObserver test_observer;

  matching_engine.doProcessOrderRequest(test_order_request_sell, test_passive_order_book, test_observer);
  matching_engine.doProcessOrderRequest(test_order_request_buy, test_passive_order_book, test_observer);

  BOOST_CHECK(test_observer.client_order_responses_.size() == 3);

  const auto &client_order_response1 = test_observer.client_order_responses_[0];
  BOOST_CHECK_EQUAL(client_order_response1.client_, test_order_request_sell_clone.client_);
  BOOST_CHECK_EQUAL(client_order_response1.client_order_id_, test_order_request_sell_clone.cln_order_id_);
  BOOST_CHECK_EQUAL(client_order_response1.instrument_, test_order_request_sell_clone.instrument_);
  BOOST_CHECK_EQUAL(client_order_response1.order_price_, test_order_request_sell_clone.price_);
  BOOST_CHECK_EQUAL(client_order_response1.size_, test_order_request_sell_clone.size_);
  BOOST_CHECK(client_order_response1.request_result_ == OrderRequestResult::ACK);
  BOOST_CHECK(client_order_response1.validation_response_ == ValidationResponse::NO_ERROR);

  const auto &client_order_response2 = test_observer.client_order_responses_[1];
  BOOST_CHECK_EQUAL(client_order_response2.client_, test_order_request_buy_clone.client_);
  BOOST_CHECK_EQUAL(client_order_response2.client_order_id_, test_order_request_buy_clone.cln_order_id_);
  BOOST_CHECK_EQUAL(client_order_response2.instrument_, test_order_request_buy_clone.instrument_);
  BOOST_CHECK_EQUAL(client_order_response2.order_price_, test_order_request_buy_clone.price_);
  BOOST_CHECK_EQUAL(client_order_response2.size_, test_order_request_buy_clone.size_);
  BOOST_CHECK(client_order_response2.request_result_ == OrderRequestResult::ACK);
  BOOST_CHECK(client_order_response2.validation_response_ == ValidationResponse::NO_ERROR);

  const auto &client_order_response3 = test_observer.client_order_responses_[2];
  BOOST_CHECK_EQUAL(client_order_response3.client_, test_order_request_buy_clone.client_);
  BOOST_CHECK_EQUAL(client_order_response3.client_order_id_, test_order_request_buy_clone.cln_order_id_);
  BOOST_CHECK_EQUAL(client_order_response3.instrument_, test_order_request_buy_clone.instrument_);
  BOOST_CHECK_EQUAL(client_order_response3.order_price_, test_order_request_buy_clone.price_);
  BOOST_CHECK_EQUAL(client_order_response3.size_, test_order_request_buy_clone.size_);
  BOOST_CHECK(client_order_response3.request_result_ == OrderRequestResult::NACK);
  BOOST_CHECK(client_order_response3.validation_response_ == ValidationResponse::SELF_MATCH);

  // Expect buy violating self-match is not placed to bid queue as passive
  BOOST_CHECK_EQUAL(test_passive_order_book.getBidOrderQueue().size(), 0);

  const auto &[best_ask_price, best_ask_order_queue] {*test_passive_order_book.getAskOrderQueue().begin()};
  BOOST_CHECK_EQUAL(best_ask_price, DEFAULT_TEST_ORDER_PRICE);
  BOOST_CHECK_EQUAL(best_ask_order_queue.size(), 1);

}

} // end of namespace

BOOST_AUTO_TEST_SUITE_END()
