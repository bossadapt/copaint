#include "net_common.h"
#include <cstddef>
#include <mutex>
#include <utility>

namespace olc {
namespace net {
template <typename T> class tsqueue {
public:
  tsqueue() = default;
  tsqueue(const tsqueue<T> &) = delete;
  virtual ~tsqueue() { clear(); }

public:
  const T &front() {
    std::scoped_lock lock(muxQueue);
    return deQueue.front();
  }
  const T &back() {
    std::scoped_lock lock(muxQueue);
    return deQueue.back();
  }
  void push_back(const T &item) {
    std::scoped_lock lock(muxQueue);
    deQueue.emplace_back(std::move(item));
  }
  void push_front(const T &item) {
    std::scoped_lock lock(muxQueue);
    deQueue.emplace_front(std::move(item));
  }
  bool empty() {
    std::scoped_lock lock(muxQueue);
    return deQueue.empty();
  }
  size_t count() {
    std::scoped_lock lock(muxQueue);
    return deQueue.size();
  }
  void clear() {
    std::scoped_lock lock(muxQueue);
    deQueue.clear();
  }
  T pop_front() {
    std::scoped_lock lock(muxQueue);
    auto t = std::move(deQueue.front());
    deQueue.pop_front();
    return t;
  }

protected:
  std::mutex muxQueue;
  std::deque<T> deQueue;
};

} // namespace net

} // namespace olc