#include "state.h"

State::State(double money) : money(money) {}

double State::get_money() { return money; }
void State::update() { money -= 0.1; }
