#include "Observer.h"

Subject::Subject() {
  _observers = std::list<Observer*>();
}

void Subject::attach(Observer *o) {
  _observers.push_back(o);
}

void Subject::detach(Observer *o) {
  _observers.remove(o);
}

void Subject::notify () {
  for (Observer* o : _observers) {
    o->update(this);
  }
}