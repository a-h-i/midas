#pragma once
#include <concepts>
#include <iterator>
#include <list>

/**
 * Simple class for adding observability.
 *
 */
template <typename HandlerT> class EventSubject {
  std::list<HandlerT> listeners;

public:
  typedef decltype(listeners)::const_iterator ListenerIdType;

  ListenerIdType add_listener(HandlerT observer) {
    listeners.push_back(observer);
    return std::prev(listeners.cend());
  }

  template <class... Args> void notify(Args &&...args) {
    std::for_each(std::begin(listeners), std::end(listeners),
                  [&](HandlerT obs) { obs(std::forward<Args>(args)...); });
  }

  void remove_listener(ListenerIdType listener) { listeners.erase(listener); }
};
