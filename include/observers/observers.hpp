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
  

  decltype(auto) add_listener(HandlerT observer) {
    listeners.push_back(observer);
    return std::prev(listeners.cend());
  }
  
  template <class... Args> void notify(Args &&...args) {
    std::for_each(
        std::begin(listeners), std::end(listeners),
        [&](HandlerT obs) { obs(std::forward<Args>(args)...); });
  }

  template <typename ListenerId>
  void remove_listener(ListenerId listener) { listeners.erase(listener); }

private:
  std::list<HandlerT> listeners;
};
