#include "utility.h"

double sigmoid(const double x) {
  return 1.0 / (1.0 + std::exp(-x));
}
