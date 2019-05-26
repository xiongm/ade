#include <bits/stdc++.h>

using namespace std;

class ThisEvent;
class ThatEvent;
struct EventHandler {
  virtual void handle(ThisEvent *) = 0;
  virtual void handle(ThatEvent *) = 0;
  ~EventHandler() = default;
};

struct Event {

  virtual void dispatch(EventHandler * eh) = 0;
  ~Event() = default;
};

template<class T>
struct EventDispatcher : public Event {

  void dispatch(EventHandler * eh) override {
    eh->handle(static_cast<T*>(this));
  }

};



struct SimpleEventHandler : public EventHandler {
  void handle(ThisEvent *) {
    cout << "handle this event" << endl;

  }

  void handle(ThatEvent *) {
    cout << "handle that event" << endl;
  }
};

struct ThisEvent : public EventDispatcher<ThisEvent> {

};

struct ThatEvent : public EventDispatcher<ThatEvent> {

};




int main(int argc, char *argv[])
{
  vector<unique_ptr<Event>> events;

  events.emplace_back(make_unique<ThisEvent>());
  events.emplace_back(make_unique<ThatEvent>());
  events.emplace_back(make_unique<ThisEvent>());


  SimpleEventHandler handler;

  for (auto & p : events) {
    p->dispatch(&handler);
  }

  return 0;
}

