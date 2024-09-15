#pragma once
#include <concepts>
#include <iterator>
#include <list>

/**
 * Simple class for adding observability.
 *
 */
template <typename HandlerT> class EventSubject {

public:
  struct EventObserver {
    std::string observerId;
    HandlerT observer;
  };

  template <class... Args>
  decltype(auto) add_listener(EventObserver observer) {
    listeners.push_back(observer);
    return std::prev(listeners.cend());
  }
  template <class... Args>
  decltype(auto) emplace_listener(Args &&...args) {
    return add_listener(EventObserver(std::forward<Args>(args)...));
  }
  template <class... Args> void notify(Args &&...args) {
    std::for_each(
        std::begin(listeners), std::end(listeners),
        [&](EventObserver obs) { obs.observer(std::forward<Args>(args)...); });
  }

  template <typename ListenerId>
  void remove_listener(ListenerId listener) { listeners.erase(listener); }

private:
  std::list<EventObserver> listeners;
};
