#pragma once

#include <queue>
#include <map>
#include <unordered_map>
#include <functional>
#include <optional>

#include "types.h"
#include "matching/passive_order.h"

namespace codetest::matching_engine_sim {

template<typename OrderExt>
class PassiveOrder;

template<typename OrderExt = void>
class PassiveOrderBook final {
 public:

  using PassiveOrderPtr = std::shared_ptr<PassiveOrder<OrderExt>>;
  // vector used instead of queue
  // 1. queue by default uses deque which no guarantee objects are in contiguous memory (cache locality)
  // 2. instead of employing vector as underlying data structure for queue, using vector directly offers
  // opportunities to provide more features such as skipping particular order in matching process
  using OrderContainer = std::vector<PassiveOrderPtr>;

  PassiveOrderBook() = default;
  PassiveOrderBook(const PassiveOrderBook &) = default;
  PassiveOrderBook(PassiveOrderBook &&) noexcept = default;
  PassiveOrderBook &operator=(const PassiveOrderBook &) = default;
  PassiveOrderBook &operator=(PassiveOrderBook &&) noexcept = default;
  ~PassiveOrderBook() = default;

  [[nodiscard]] auto &getAskOrderQueue() { return ask_orders_; }
  [[nodiscard]] auto &getBidOrderQueue() { return bid_orders_; }
  [[nodiscard]] const auto &getAskOrderQueue() const { return ask_orders_; }
  [[nodiscard]] const auto &getBidOrderQueue() const { return bid_orders_; }

  void cancelClientOrder(const ClientType &client, const OrderIDType &order_id);

  void placePassiveOrder(const ClientType &client,
                         const OrderIDType &cln_order_id,
                         const OrderType &order_type,
                         const OrderSide &order_side,
                         const PriceType &price,
                         const SizeType &size,
                         const std::shared_ptr<OrderExt> &custom_fields);

  [[nodiscard]]
  bool isOrderExist(const ClientType &client, const OrderIDType &order_id) const;

  [[nodiscard, maybe_unused]]
  PassiveOrderPtr getEngineOrderFromCache(const ClientType &client, const OrderIDType &order_id);

 private:
  // using map for key based (Price) ordering
  std::map<PriceType, OrderContainer> ask_orders_{};
  std::map<PriceType, OrderContainer, std::greater<PriceType>> bid_orders_{};

  // Provides hash-based ClientID-OrderID to EngineOrderRef lookup
  // Crucial to avoid linear search for order amend and cancel request
  std::unordered_map<ClientType, std::unordered_map<OrderIDType, PassiveOrderPtr>> client_orders_map_{};
};

template<typename OrderExt>
void PassiveOrderBook<OrderExt>::cancelClientOrder(const ClientType &client,
                                                   const OrderIDType &order_id) {
  auto client_itr = client_orders_map_.find(client);
  if (client_itr != client_orders_map_.end()) {
    auto &[_, order_id_map] = *client_itr;
    auto order_itr = order_id_map.find(order_id);
    if (order_itr != order_id_map.end()) {
      auto &passive_order = order_itr->second;
      passive_order->remaining_size_ = 0;
      order_id_map.erase(order_itr);
    }
  }
}

template<typename OrderExt>
void PassiveOrderBook<OrderExt>::placePassiveOrder(const ClientType &client,
                                                   const OrderIDType &cln_order_id,
                                                   const OrderType &order_type,
                                                   const OrderSide &order_side,
                                                   const PriceType &price,
                                                   const SizeType &size,
                                                   const std::shared_ptr<OrderExt> &custom_fields) {
  if (size > 0 && order_type != OrderType::MARKET) {
    PassiveOrderPtr ptr = std::make_shared<PassiveOrder<OrderExt>>(client, cln_order_id, size, custom_fields);

    if (order_side == OrderSide::BUY)
      bid_orders_[price].push_back(ptr);
    else
      ask_orders_[price].push_back(ptr);

    client_orders_map_[client][ptr->cln_order_id_] = std::move(ptr);
  }
}

template<typename OrderExt>
[[nodiscard]] bool PassiveOrderBook<OrderExt>::isOrderExist(const ClientType &client,
                                                            const OrderIDType &order_id) const {
  auto itr = client_orders_map_.find(client);
  if (itr == client_orders_map_.end()) return false;
  const auto &[_, order_id_map] {*itr};
  return order_id_map.find(order_id) != order_id_map.end();
}

template<typename OrderExt>
[[nodiscard, maybe_unused]]
auto PassiveOrderBook<OrderExt>::getEngineOrderFromCache(
    const ClientType &client,
    const OrderIDType &order_id) -> PassiveOrderBook<OrderExt>::PassiveOrderPtr {

  auto itr = client_orders_map_.find(client);
  if (itr == client_orders_map_.end()) return nullptr;

  const auto &[_, order_id_map] {*itr};
  const auto order_id_map_itr = order_id_map.find(order_id);
  if (order_id_map_itr != order_id_map.end()) return order_id_map_itr->second;

  return nullptr;

}

} // end of namespace