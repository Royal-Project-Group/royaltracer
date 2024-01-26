//
// Created by nikla on 22.10.2023.
//

#ifndef ROYAL_TRACER_OBSERVER_H
#define ROYAL_TRACER_OBSERVER_H

//#include "Subject.h"
#include <list>

class Subject;

class Observer {
public:
  virtual ~Observer() = 0;
  virtual void update(Subject* theChangedSubject) = 0;
protected:
  Observer() = default;
};

class Subject {
public:
  virtual ~Subject() = 0;

  virtual void attach(Observer* o);
  virtual void detach(Observer* o);
  virtual void notify();
protected:
  Subject();
private:
  std::list<Observer*> _observers;
};

#endif //ROYAL_TRACER_OBSERVER_H
