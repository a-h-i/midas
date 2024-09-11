#pragma once
#include <concepts>
#include <vector>

/**
 * Simple class for adding observability.
 * 
 */
template <std::invocable HandlerT> class EventSubject {

public:
  struct EventObserver {
    std::string observerId;
    HandlerT observer;
  };
  void add_listener(EventObserver observer) {
    listeners.push_back(observer);
  }
  template <class... Args> void emplace_listener(Args &&...args) {
    add_listener(EventObserver(std::forward<Args>(args)...));
  }
  template <class... Args> void notify(Args &&...args) {
    std::for_each(std::begin(listeners), std::end(listeners),
                  [&](EventObserver obs) {
                    obs.observer(std::forward<Args>(args)...);
                  });
  }

private:
  std::vector<EventObserver> listeners;
};
