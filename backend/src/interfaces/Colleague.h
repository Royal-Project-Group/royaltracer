//
// Created by nikla on 22.10.2023.
//

#ifndef ROYAL_TRACER_COLLEAGUE_H
#define ROYAL_TRACER_COLLEAGUE_H

class RoyalMediator;

class Colleague {
public:
  RoyalMediator* mediator;
  explicit Colleague(RoyalMediator *mediator);
};


#endif //ROYAL_TRACER_COLLEAGUE_H
