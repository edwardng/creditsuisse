#pragma once

#include <atomic>
#include <unordered_set>
#include <set>
#include <vector>
#include <thread>
#include <mutex>

#include "matching/matching_algo.hpp"
#include "matching/passive_order_book.hpp"
#include "matching/validators/matching_validators.hpp"
#include "matching/validators/new_order_request_validators.hpp"
#include "matching/validators/cancel_request_validators.hpp"
#include "interface/i_matching_algo.h"
#include "interface/i_matching_engine.h"
#include "events/client_order_request.h"
#include "external/i_client.h"

namespace codetest::matching_engine_sim {

namespace {

constexpr static unsigned DEFAULT_ORDER_QUEUE_SIZE = 1024;

template<typename OrderExt = void>
struct MatchingInstrument {
  MatchingInstrument() {
    request_queue_.reserve(DEFAULT_ORDER_QUEUE_SIZE);
  }

  PassiveOrderBook<OrderExt> passive_order_book_{};
  std::vector<ClientOrderRequest<OrderExt>> request_queue_{};
  std::mutex mutex_{};
};

template<typename OrderExt = void>
class OrderQueueProcessor {
 public:
  OrderQueueProcessor(
      const std::shared_ptr<IMatchingAlgo<OrderExt>> &matching_algo,
      const std::shared_ptr<IEngineEventObserver> &observer)
      : matching_algo_(matching_algo), observer_(observer), in_operation_(true) {}

  template<typename F>
  void addMatchingInstrument(F &&matching_instrument_ptr);

  void processOrderQueue();
  void terminate();

 private:
  std::vector<std::shared_ptr<MatchingInstrument<OrderExt>>> matching_instruments_;
  const std::shared_ptr<IMatchingAlgo<OrderExt>> matching_algo_{};
  std::shared_ptr<IEngineEventObserver> observer_{};
  std::atomic<bool> in_operation_{};
};

template<typename OrderExt>
template<typename F>
void OrderQueueProcessor<OrderExt>::addMatchingInstrument(F &&matching_instrument_ptr) {
  matching_instruments_.push_back(std::forward<F>(matching_instrument_ptr));
}

template<typename OrderExt>
void OrderQueueProcessor<OrderExt>::processOrderQueue() {

  std::vector<ClientOrderRequest<OrderExt>> client_order_request_queue;
  client_order_request_queue.reserve(DEFAULT_ORDER_QUEUE_SIZE);

  while (in_operation_) {

    for (auto &matching_instrument : matching_instruments_) {
      {
        std::lock_guard<std::mutex> _{matching_instrument->mutex_};
        if (!matching_instrument->request_queue_.empty()) {
          matching_instrument->request_queue_.swap(client_order_request_queue);
        }
      }

      if (client_order_request_queue.empty()) continue;

      for (auto itr = client_order_request_queue.begin(); itr != client_order_request_queue.end(); itr++) {
        ClientOrderRequest<OrderExt> &order_request = *itr;
        matching_algo_->doProcessOrderRequest(order_request, matching_instrument->passive_order_book_, *observer_);
      }

      client_order_request_queue.clear();
    }

  }
}

template<typename OrderExt>
void OrderQueueProcessor<OrderExt>::terminate() {
  in_operation_.store(false);
}

} // end of namespace

template<typename OrderExt = void>
struct DefaultMatchingEngine : public IMatchingEngine<OrderExt> {

  DefaultMatchingEngine(const std::uint8_t &number_of_thread,
                        const std::set<InstrumentType> &instruments,
                        const std::shared_ptr<IEngineEventObserver> &observer);

  void doOrderRequest(const ClientOrderRequest<OrderExt> &client_order_request) override;
  void terminate() override;

  using MatchValidators = Validators<OrderExt, NoSelfMatchValidator<OrderExt>>;
  using NewValidators = Validators<OrderExt, NoSuchOrderInsertValidator<OrderExt>>;
  using CancelValidators = Validators<OrderExt, NoSuchOrderCancelValidator<OrderExt>>;
  using DefaultMatchingAlgo = PriceTimePriorityMatching<OrderExt, MatchValidators, NewValidators, CancelValidators>;

 private:

  std::shared_ptr<DefaultMatchingAlgo> matching_algo_{};

  std::unordered_map<InstrumentType, std::shared_ptr<MatchingInstrument<OrderExt>>> matching_instruments_{};

  std::vector<std::unique_ptr<OrderQueueProcessor<OrderExt>>> order_queue_processors_{};
  std::vector<std::unique_ptr<std::thread>> processor_threads_{};
};

template<typename OrderExt>
DefaultMatchingEngine<OrderExt>::DefaultMatchingEngine(
    const std::uint8_t &number_of_thread,
    const std::set<InstrumentType> &instruments,
    const std::shared_ptr<IEngineEventObserver> &observer) {

  if (number_of_thread == 0) {
    throw std::invalid_argument("number of thread cannot be 0");
  }

  matching_algo_ = std::make_shared<DefaultMatchingAlgo>();
  for (std::uint8_t cnt = 0; cnt < number_of_thread; cnt++) {
    order_queue_processors_.emplace_back(
        std::move(std::make_unique<OrderQueueProcessor<OrderExt>>(matching_algo_, observer)));
  }

  std::uint8_t thread_index = static_cast<std::uint8_t>(number_of_thread - 1);

  for (const InstrumentType &inst : instruments) {
    auto [itr, ok] = matching_instruments_.emplace(inst, std::move(std::make_shared<MatchingInstrument<OrderExt>>()));
    if (++thread_index == number_of_thread) {
      thread_index = 0;
    }
    order_queue_processors_[thread_index]->addMatchingInstrument(itr->second);
  }

  for (thread_index = 0; thread_index < number_of_thread; thread_index++) {
    processor_threads_.emplace_back(
        std::move(
            std::make_unique<std::thread>(
                &OrderQueueProcessor<OrderExt>::processOrderQueue, order_queue_processors_[thread_index].get())));
  }

}

template<typename OrderExt>
void DefaultMatchingEngine<OrderExt>::doOrderRequest(const ClientOrderRequest<OrderExt> &client_order_request) {

  const auto &instrument = client_order_request.instrument_;
  if (auto itr = matching_instruments_.find(instrument); itr != matching_instruments_.end()) {
    auto &matching_instrument = itr->second;
    auto &request_queue = matching_instrument->request_queue_;
    auto client_order_request_clone = client_order_request;

    std::lock_guard<std::mutex> _{matching_instrument->mutex_};
    request_queue.emplace_back(std::move(client_order_request_clone));
  }

}

template<typename OrderExt>
void DefaultMatchingEngine<OrderExt>::terminate() {
  for (auto &processor : order_queue_processors_) {
    processor->terminate();
  }
  for (auto &t : processor_threads_) {
    t.get()->join();
  }
}

} // end of namespace