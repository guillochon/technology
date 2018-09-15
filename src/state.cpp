#include "entity.h"
#include "state.h"
#include <algorithm>
#include <cmath>
#include <functional>

State::State(double money, long epoch, double x_size, double y_size)
    : money(money), epoch(epoch), x_size(x_size), y_size(y_size) {
  newx_dist = std::uniform_real_distribution<double>(0.0, x_size);
  newy_dist = std::uniform_real_distribution<double>(0.0, y_size);
  entities = std::vector<Entity>();
}

double State::money_value() const { return money; }

long State::epoch_value() const { return epoch; }

double State::x_size_value() const { return x_size; }

double State::y_size_value() const { return y_size; }

std::string State::date_str() const {
  return std::to_string(0.1 * std::floor(10.0 * epoch / 86400.0 / 365.0));
}

void State::update() {
  constexpr double year = 86400.0 * 365.0;
  constexpr double mate_mood = 5.0;

  money -= 0.1;
  epoch += 86400;

  std::for_each(entities.begin(), entities.end(), std::mem_fn(&Entity::move));

  std::for_each(entities.begin(), entities.end(),
                std::mem_fn(&Entity::adjust_needs));

  int old_num = num_entities();

  std::vector<Entity> babies;

  for (int i = 0; i < d2s.size(); i++) {
    for (int j = i + 1; j < d2s[i].size(); j++) {
      if (!entities[i].alive_value() || !entities[j].alive_value()) {
        if (!entities[i].alive_value()) {
          std::cout << "entities parent epoch: "
                    << entities[i].parent_value()->epoch_value() << std::endl;
          std::cout << "time since death: " << entities[i].time_since_death()
                    << std::endl;
        }
        if (!entities[j].alive_value()) {
          std::cout << "time since death: " << entities[j].time_since_death()
                    << std::endl;
        }
        continue;
      }
      d2s[i][j] = pow(entities[i].x_value() - entities[j].x_value(), 2) +
                  pow(entities[i].y_value() - entities[j].y_value(), 2);
      affinities[i][j] += 0.1 * (5.0 - d2s[i][j]);
      affinities[i][j] = std::max(std::min(affinities[i][j], 1.0), 0.0);

      if (affinities[i][j] > 0.8) {
        entities[i].interact(entities.at(j));
        std::cout << i << " and " << j << " interacted." << std::endl;
        if (entities[i].mood_value() > mate_mood &&
            entities[j].mood_value() > mate_mood &&
            entities[i].age_value() > year && entities[j].age_value() > year) {
          babies.push_back(entities[i].mate(entities.at(j)));
          std::cout << i << " and " << j << " mated." << std::endl;
        }
      }
    }
  }

  entities.insert(entities.end(), std::make_move_iterator(babies.begin()),
                  std::make_move_iterator(babies.end()));

  std::for_each(entities.begin(), entities.end(),
                std::mem_fn(&Entity::check_for_death));

  entities.erase(std::remove_if(entities.begin(), entities.end(),
                                [](const Entity &x) {
                                  return !x.alive_value() &&
                                         x.time_since_death() > year;
                                }),
                 entities.end());

  int new_num = num_entities();

  if (new_num != old_num) {
    std::cout << "entities after update: " << num_entities() << std::endl;
    resize_pairwise();
  }
}

void State::add_entity(const std::string &name, double x, double y) {
  if (x == 0.0)
    x = newx_dist(gen);
  if (y == 0.0)
    y = newy_dist(gen);
  entities.emplace_back(this, name, x, y);

  resize_pairwise();
}

void State::resize_pairwise() {
  // Resize pairwise arrays.
  std::cout << "Resizing pairwise to " << entities.size() << "." << std::endl;
  d2s = std::vector<std::vector<double>>(
      entities.size(), std::vector<double>(entities.size(), 1));

  affinities = std::vector<std::vector<double>>(
      entities.size(), std::vector<double>(entities.size(), 1));
}

std::default_random_engine *State::get_gen() { return &gen; }

const int State::num_entities() const { return entities.size(); }

const std::vector<Entity> *State::entities_value() const { return &entities; }
