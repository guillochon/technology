#include "entity.h"
#include "state.h"
#include "utility.h"
#include <algorithm>
#include <cmath>
#include <functional>
#include <limits>

State::State(double money, long epoch, double x_size, double y_size)
    : money(money), epoch(epoch), x_size(x_size), y_size(y_size) {
  std::random_device rd;
  gen = std::default_random_engine(rd());
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
  money -= 0.1;
  epoch += 86400;

  std::for_each(entities.begin(), entities.end(), std::mem_fn(&Entity::move));

  std::for_each(entities.begin(), entities.end(),
                std::mem_fn(&Entity::adjust_needs));

  int old_num = num_entities();

  std::vector<Entity> babies;

  for (int i = 0; i < d2s.size(); i++) {
    for (int j = i + 1; j < d2s[i].size(); j++) {
      if (!entities[i].alive_value() || !entities[j].alive_value())
        continue;
      d2s[i][j] = pow(entities[i].x_value() - entities[j].x_value(), 2) +
                  pow(entities[i].y_value() - entities[j].y_value(), 2);
      affinities[i][j] += 0.1 * (5.0 - d2s[i][j]);
      affinities[i][j] = std::max(std::min(affinities[i][j], 1.0), 0.0);

      if (affinities[i][j] > 0.8) {
        entities[i].interact(entities.at(j));
        std::cout << i << " and " << j << " interacted." << std::endl;

        if (entities[i].can_eat(entities.at(j))) {
          // Eating.
          bool ieatj = entities[i].energy_value() > entities[j].energy_value();
          int eater = ieatj ? i : j;
          if (entities[eater].is_hungry()) {
            int target = ieatj ? j : i;
            entities[eater].consume(entities.at(target));
            std::cout << i << " ate " << j << "!" << std::endl;
          }
        } else if (entities[i].will_mate() && entities[j].will_mate() &&
                   entities[i].can_mate(entities.at(j))) {
          // Mating.
          babies.push_back(entities[i].mate(entities.at(j)));
          std::cout << i << " and " << j << " mated." << std::endl;
        }
      }
    }
  }

  entities.insert(entities.end(), std::make_move_iterator(babies.begin()),
                  std::make_move_iterator(babies.end()));

  resize_pairwise();

  // Determine which entities we are removing.
  std::vector<int> to_erase;
  for (int i = 0; i < num_entities(); i++) {
    if (!entities[i].alive_value() &&
        entities[i].time_since_death() > Entity::corpse_lifetime) {
      to_erase.push_back(i);
    }
  }

  // Erase these entities, and associated array structures.
  entities.erase(
      ToggleIndices(entities, std::begin(to_erase), std::end(to_erase)),
      entities.end());

  for (int i = 0; i < d2s.size(); i++) {
    d2s[i].erase(
        ToggleIndices(d2s[i], std::begin(to_erase), std::end(to_erase)),
        d2s[i].end());
    affinities[i].erase(
        ToggleIndices(affinities[i], std::begin(to_erase), std::end(to_erase)),
        affinities[i].end());
  }
  d2s.erase(ToggleIndices(d2s, std::begin(to_erase), std::end(to_erase)),
            d2s.end());
  affinities.erase(
      ToggleIndices(affinities, std::begin(to_erase), std::end(to_erase)),
      affinities.end());

  // Kill entities marked to die.
  for (int i = 0; i < num_entities(); i++) {
    if (entities[i].will_die_value())
      entities[i].kill();
  }

  // Check if any entities met criteria for death.
  std::for_each(entities.begin(), entities.end(),
                std::mem_fn(&Entity::check_for_death));

  int new_num = num_entities();

  if (new_num != old_num) {
    std::cout << "Entity count changed to: " << new_num << std::endl
              << std::flush;
  }
}

double State::smallest_non_negative_or_NaN(double a, double b) const {
  if (a <= 0.0 || std::isnan(a)) {
    if (b >= 0.0 && !std::isnan(b))
      return b;
    return 0.0;
  } else {
    return a;
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
  // std::cout << "Resizing pairwise to " << entities.size() << "." <<
  // std::endl;
  d2s.resize(entities.size());
  affinities.resize(entities.size());
  for (int i = 0; i < entities.size(); i++) {
    d2s[i].resize(entities.size());
    affinities[i].resize(entities.size());
  }
}

std::default_random_engine *State::get_gen() { return &gen; }

const int State::num_entities() const { return entities.size(); }

const std::vector<Entity> *State::entities_value() const { return &entities; }

const std::vector<std::vector<double>> *State::d2s_value() const {
  return &d2s;
}

void State::intecept_trajectory(const Entity *projectile, const Entity *target,
                                double travel_time, double &Vhx,
                                double &Vhy) const {
  const double &t = travel_time;
  double Vtx, Vty, Ptx, Pty, Phx, Phy, sh;

  Phx = projectile->x_value();
  Phy = projectile->y_value();
  sh = projectile->max_speed_value();
  Ptx = target->x_value();
  Pty = target->y_value();
  Vtx = target->px_value();
  Vty = target->py_value();

  Vhx = (Ptx - Phx + (t * Vtx)) / (t * sh);
  Vhy = (Pty - Phy + (t * Vty)) / (t * sh);
}

double State::entity_intercept_time(const Entity *actor,
                                    const Entity *target) const {
  // Compute intersection.
  double a, b, c, t1, t2, st;
  double Phx = actor->x_value();
  double Phy = actor->y_value();
  double sh = actor->max_speed_value();
  double Ptx = target->x_value();
  double Pty = target->y_value();
  double Vtx = target->px_value();
  double Vty = target->py_value();

  st = std::sqrt(Vtx * Vtx + Vty * Vty);

  a = (Vtx * Vtx) + (Vty * Vty) - (sh * sh);
  b = 2 * ((Ptx * Vtx) + (Pty * Vty) - (Phx * Vtx) - (Phy * Vty));
  c = (Ptx * Ptx) + (Pty * Pty) + (Phx * Phx) + (Phy * Phy) - (2 * Phx * Ptx) -
      (2 * Phy * Pty);

  t1 = (-b + sqrt((b * b) - (4 * a * c))) / (2 * a);
  t2 = (-b - sqrt((b * b) - (4 * a * c))) / (2 * a);

  return smallest_non_negative_or_NaN(t1, t2);
}

const Entity *State::nearest_target(const Entity *actor,
                                    double &time_of_travel,
                                    std::string looking_for) const {
  time_of_travel = std::numeric_limits<double>::infinity();
  const Entity *target = NULL;
  double t;

  for (int i = 0; i < d2s.size(); i++) {
    if (&entities[i] != actor)
      continue;
    for (int j = i + 1; j < d2s[i].size(); j++) {
      if (&entities[j] != actor) {
        if (looking_for == "mate") {
          if (!actor->can_mate(entities[j]))
            continue;
        } else {
          if (!actor->can_eat(entities[j]))
            continue;
        }

        t = entity_intercept_time(actor, &entities[j]);

        if (t > 0.0 && t < time_of_travel) {
          time_of_travel = t;
          target = &entities[j];
        }
      }
    }
  }

  return target;
}

void State::clear_entity_target(int i) { entities[i].clear_current_target(); }
