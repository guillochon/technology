#include "technology.h"

Technology::Technology(double cost) : cost(cost) {}

double Technology::cost_value() const { return cost; }
