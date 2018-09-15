#ifndef UTILITY_H
#define UTILITY_H

#include <list>
#include <algorithm>
#include <cmath>

template <typename Cont, typename It>
auto ToggleIndices(Cont &cont, It beg, It end) -> decltype(std::end(cont)) {
  int helpIndx(0);
  return std::stable_partition(
      std::begin(cont), std::end(cont),
      [&](decltype(*std::begin(cont)) const &val) -> bool {
        return std::find(beg, end, helpIndx++) == end;
      });
}

double sigmoid(const double x) {
  return 1.0 / (1.0 + std::exp(-x));
}

#endif
