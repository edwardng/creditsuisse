#pragma once

namespace codetest::matching_engine_sim {

template<typename OrderExt>
class ClientOrderRequest;

template<typename OrderExt>
struct IMatchingEngine {
  IMatchingEngine() = default;
  IMatchingEngine(IMatchingEngine &&) noexcept = default;
  IMatchingEngine &operator=(const IMatchingEngine &) = default;
  IMatchingEngine &operator=(IMatchingEngine &&) noexcept = default;
  virtual ~IMatchingEngine() = default;

  virtual void doOrderRequest(const ClientOrderRequest<OrderExt> &client_order_request) = 0;
  virtual void terminate() = 0;
};

} // end of namespace