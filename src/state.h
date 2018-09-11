#ifndef STATE_H
#define STATE_H

#include <iostream>
#include <list>
#include <random>

#include "entity.h"

class Entity;
class State {

private:
  double money;
  long date;
  double x_size, y_size;
  std::default_random_engine gen;
  std::list<Entity> entities;

public:
  State(double money, long date, double x_size, double y_size);
  double money_value() const;
  long date_value() const;
  std::string date_str() const;
  std::default_random_engine *get_gen();
  void update();
  void add_entity();
  double x_size_value() const;
  double y_size_value() const;
  const std::list<Entity> *get_entities() const;
};

#endif
