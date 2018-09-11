#include "state.h"
#include <algorithm>
#include <cmath>
#include <functional>

State::State(double money, long date, double x_size, double y_size)
    : money(money), date(date), x_size(x_size), y_size(y_size) {}

double State::money_value() const { return money; }

long State::date_value() const { return date; }

double State::x_size_value() const { return x_size; }

double State::y_size_value() const { return y_size; }

std::string State::date_str() const {
  return std::to_string(0.1 * std::floor(10.0 * date / 86400.0 / 365.0));
}

void State::update() {
  money -= 0.1;
  date += 86400;

  std::for_each(entities.begin(), entities.end(), std::mem_fn(&Entity::move));
}

void State::add_entity() {
  double x = 320.0;
  double y = 240.0;
  Entity new_entity = Entity(this, x, y);
  entities.push_back(new_entity);
}

std::default_random_engine *State::get_gen() { return &gen; }

const std::list<Entity> *State::get_entities() const {
  return &(this->entities);
}
