#pragma once

#include <array>
#include <vector>
#include <limits>
#include <type_traits>
#include <functional>

#include "types.h"
#include "matching/passive_order.h"
#include "matching/passive_order_book.hpp"
#include "matching/validators/validators.hpp"
#include "matching/validators/new_order_request_validators.hpp"
#include "interface/i_engine_event_observer.h"
#include "interface/i_matching_algo.h"

namespace codetest::matching_engine_sim {

class IEngineEventObserver;

using NoOrderExt = void;
using NoValidator = Validators<NoOrderExt>;

template<
    typename OrderExt = NoOrderExt,
    typename MatchValidators = NoValidator,
    typename NewValidators = NoValidator,
    typename CancelValidators = NoValidator>
class PriceTimePriorityMatching : public IMatchingAlgo<OrderExt> {
 public:
  PriceTimePriorityMatching();
  PriceTimePriorityMatching(const PriceTimePriorityMatching &) = default;
  PriceTimePriorityMatching(PriceTimePriorityMatching &&) noexcept = default;
  virtual PriceTimePriorityMatching &operator=(const PriceTimePriorityMatching &) = default;
  virtual PriceTimePriorityMatching &operator=(PriceTimePriorityMatching &&) noexcept = default;
  virtual ~PriceTimePriorityMatching() = default;

  void doProcessOrderRequest(
      ClientOrderRequest<OrderExt> &order_request,
      PassiveOrderBook<OrderExt> &passive_order_book,
      IEngineEventObserver &observer) override;

 private:
  using RequestHandler = void (PriceTimePriorityMatching::*)(ClientOrderRequest<OrderExt> &order_request,
                                                             PassiveOrderBook<OrderExt> &passive_order_book,
                                                             IEngineEventObserver &observer);

  void doProcessNewOrderRequest(ClientOrderRequest<OrderExt> &order_request,
                                PassiveOrderBook<OrderExt> &passive_order_book,
                                IEngineEventObserver &observer);

  void doProcessCancelOrderRequest(ClientOrderRequest<OrderExt> &order_request,
                                   PassiveOrderBook<OrderExt> &passive_order_book,
                                   IEngineEventObserver &observer);

  std::array<RequestHandler, static_cast<std::size_t>(OrderAction::_ACTION_SIZE_)> request_handlers_;

  const MatchValidators match_validators_{};
  const NewValidators new_validators_{};
  const CancelValidators cancel_validators_{};
};

namespace {
template<typename OrderExt, typename MatchValidators, typename MatchOrderPriceQueues>
[[nodiscard]] ValidationResponse executeOrder(ClientOrderRequest<OrderExt> &order_request,
                                              MatchOrderPriceQueues &&match_order_queues,
                                              PassiveOrderBook<OrderExt> &passive_order_book,
                                              const InstrumentType &instrument,
                                              IEngineEventObserver &observer,
                                              const MatchValidators &match_validators) {

  // We are leveraging on order queue key comparator to perform order price comparison
  static_assert(std::is_same_v<decltype(match_order_queues.key_comp()), std::greater<PriceType>> ||
                    std::is_same_v<decltype(match_order_queues.key_comp()), std::less<PriceType>>,
                "Strict weak ordering required for key (Price) comparison in matching");

  if (order_request.order_type_ == OrderType::MARKET) {
    if (order_request.side_ == OrderSide::BUY) {
      order_request.price_ = std::numeric_limits<PriceType>::max();
    } else {
      order_request.price_ = std::numeric_limits<PriceType>::min();
    }
  }

  ValidationResponse current_validation{ValidationResponse::NO_ERROR};

  auto match_order_queues_itr{match_order_queues.begin()};

  // Iterating price levels
  while (order_request.size_ > 0 &&
      match_order_queues_itr != match_order_queues.end() &&
      (current_validation == ValidationResponse::NO_ERROR
          || current_validation == ValidationResponse::CONTINUE_WITHOUT_MATCHING)) {

    auto &[current_order_queue_price, order_queue] {*match_order_queues_itr};
    if (match_order_queues.key_comp()(order_request.price_, current_order_queue_price)) break;

    // Iterating order queue per each price
    for (auto itr = order_queue.begin(); itr != order_queue.end() && order_request.size_ > 0;) {

      auto &current_passive_order_ptr = *itr;
      PassiveOrder<OrderExt> &current_passive_order = *current_passive_order_ptr;

      if (current_passive_order.remaining_size_ == 0) {
        // Ignore and remove cancelled order in the queue
        itr = order_queue.erase(itr);
        continue;
      }

      current_validation = match_validators.validate(order_request, passive_order_book, current_passive_order);

      if (current_validation == ValidationResponse::CONTINUE_WITHOUT_MATCHING) {
        itr++;
        continue;
      } else if (current_validation != ValidationResponse::NO_ERROR) {
        return current_validation;
      }

      SizeType trade_size = current_passive_order.remaining_size_;

      if (order_request.size_ >= current_passive_order.remaining_size_) {
        itr = order_queue.erase(itr);
        passive_order_book.cancelClientOrder(
            current_passive_order.client_,
            current_passive_order.cln_order_id_);
      } else {
        current_passive_order.remaining_size_ -= order_request.size_;
        trade_size = order_request.size_;
        itr++;
      }

      observer.doTradeEvent(
          order_request.client_,
          order_request.cln_order_id_,
          current_passive_order.client_,
          current_passive_order.cln_order_id_,
          instrument,
          current_order_queue_price,
          trade_size
      );

      order_request.size_ -= trade_size;
    }

    if (order_queue.empty()) {
      match_order_queues_itr = match_order_queues.erase(match_order_queues_itr);
    } else {
      match_order_queues_itr++;
    }

  }

  return current_validation;

}
} // end of anonymous local namespace

template<typename OrderExt, typename M, typename N, typename C>
PriceTimePriorityMatching<OrderExt, M, N, C>::PriceTimePriorityMatching() {
  request_handlers_[static_cast<std::size_t>(OrderAction::NEW)] =
      &PriceTimePriorityMatching::doProcessNewOrderRequest;
  request_handlers_[static_cast<std::size_t>(OrderAction::CANCEL)] =
      &PriceTimePriorityMatching::doProcessCancelOrderRequest;
}

template<typename OrderExt, typename M, typename N, typename C>
void PriceTimePriorityMatching<OrderExt, M, N, C>::doProcessOrderRequest(
    ClientOrderRequest<OrderExt> &order_request,
    PassiveOrderBook<OrderExt> &passive_order_book,
    IEngineEventObserver &observer) {

  const auto &request_handler = request_handlers_[static_cast<std::size_t>(order_request.order_action_)];
  (this->*request_handler)(order_request, passive_order_book, observer);

}

template<typename OrderExt, typename M, typename N, typename C>
void PriceTimePriorityMatching<OrderExt, M, N, C>::doProcessNewOrderRequest(
    ClientOrderRequest<OrderExt> &order_request,
    PassiveOrderBook<OrderExt> &passive_order_book,
    IEngineEventObserver &observer) {

  auto validation_response = new_validators_.validate(order_request, passive_order_book);

  auto request_result = (validation_response == ValidationResponse::NO_ERROR)
                        ? OrderRequestResult::ACK
                        : OrderRequestResult::NACK;

  observer.doOrderRequestResponse(
      order_request.client_,
      order_request.cln_order_id_,
      order_request.instrument_,
      order_request.price_,
      order_request.size_,
      request_result,
      validation_response);

  if (validation_response != ValidationResponse::NO_ERROR) return;

  if (order_request.side_ == OrderSide::BUY) {
    validation_response = executeOrder<OrderExt>(
        order_request,
        passive_order_book.getAskOrderQueue(),
        passive_order_book,
        order_request.instrument_,
        observer,
        match_validators_);
  } else if (order_request.side_ == OrderSide::SELL) {
    validation_response = executeOrder<OrderExt>(
        order_request,
        passive_order_book.getBidOrderQueue(),
        passive_order_book,
        order_request.instrument_,
        observer,
        match_validators_);
  }

  if (validation_response == ValidationResponse::NO_ERROR) {
    passive_order_book.placePassiveOrder(order_request.client_,
                                         order_request.cln_order_id_,
                                         order_request.order_type_,
                                         order_request.side_,
                                         order_request.price_,
                                         order_request.size_,
                                         order_request.custom_fields_);
  } else if (validation_response != ValidationResponse::NO_ERROR) {
    observer.doOrderRequestResponse(
        order_request.client_,
        order_request.cln_order_id_,
        order_request.instrument_,
        order_request.price_,
        order_request.size_,
        OrderRequestResult::NACK,
        validation_response);
  }

}

template<typename OrderExt, typename M, typename N, typename C>
void PriceTimePriorityMatching<OrderExt, M, N, C>::doProcessCancelOrderRequest(
    ClientOrderRequest<OrderExt> &order_request,
    PassiveOrderBook<OrderExt> &passive_order_book,
    IEngineEventObserver &observer) {

  auto validation_response = cancel_validators_.validate(order_request, passive_order_book);

  auto request_result = (validation_response == ValidationResponse::NO_ERROR)
                        ? OrderRequestResult::ACK
                        : OrderRequestResult::NACK;

  observer.doOrderRequestResponse(
      order_request.client_,
      order_request.cln_order_id_,
      order_request.instrument_,
      order_request.price_,
      order_request.size_,
      request_result,
      validation_response);

  if (validation_response == ValidationResponse::NO_ERROR) {
    passive_order_book.cancelClientOrder(order_request.client_, order_request.cln_order_id_);
  }

}

} // end of namespace