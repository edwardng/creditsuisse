#pragma once

namespace codetest::matching_engine_sim {

template<typename OrderExt>
class PassiveOrderBook;
template<typename OrderExt>
class ClientOrderRequest;
class IEngineEventObserver;

template<typename OrderExt = void>
struct IMatchingAlgo {
  IMatchingAlgo() = default;
  IMatchingAlgo(const IMatchingAlgo &) = default;
  IMatchingAlgo(IMatchingAlgo &&) noexcept = default;
  virtual IMatchingAlgo &operator=(const IMatchingAlgo &) = default;
  virtual IMatchingAlgo &operator=(IMatchingAlgo &&) noexcept = default;
  virtual ~IMatchingAlgo() = default;

  virtual void doProcessOrderRequest(
      ClientOrderRequest<OrderExt> &order_request,
      PassiveOrderBook<OrderExt> &passive_order_book,
      IEngineEventObserver &observer) = 0;
};

} // end of namespace