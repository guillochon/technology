#include "entity.h"
#include "state.h"
#include "utility.h"
#include <algorithm>
#include <cmath>
#include <functional>
#include <iomanip>
#include <limits>
#include <sstream>

State::State(double money, long epoch, double x_size, double y_size)
    : money(money), epoch(epoch), x_size(x_size), y_size(y_size) {
  std::random_device rd;
  random_generator = new std::default_random_engine(rd());
  newx_dist = std::uniform_real_distribution<double>(0.0, x_size);
  newy_dist = std::uniform_real_distribution<double>(0.0, y_size);
  entities = std::vector<Entity *>();
}

State::~State() {
  delete (random_generator);
  for (auto i : entities)
    delete i;
}

double State::money_value() const { return money; }

long State::epoch_value() const { return epoch; }

double State::x_size_value() const { return x_size; }

double State::y_size_value() const { return y_size; }

std::string State::date_str() const {
  std::ostringstream out;
  out << std::setprecision(3) << std::fixed << epoch / 86400.0 / 365.0;
  return out.str();
}

void State::update() {
  money -= 0.1;
  epoch += tick_time;

  std::for_each(entities.begin(), entities.end(), std::mem_fn(&Entity::move));

  std::for_each(entities.begin(), entities.end(),
                std::mem_fn(&Entity::adjust_needs));

  int old_num = num_entities();

  std::vector<Entity *> offspring;

  for (int i = 0; i < d2s.size(); i++) {
    for (int j = i + 1; j < d2s[i].size(); j++) {
      if (!entities[i]->alive_value() || !entities[j]->alive_value())
        continue;

      if (entities[i]->host_value() != NULL ||
          entities[j]->host_value() != NULL)
        continue;

      d2s[i][j] = pow(entities[i]->x_value() - entities[j]->x_value(), 2) +
                  pow(entities[i]->y_value() - entities[j]->y_value(), 2);
      affinities[i][j] +=
          0.1 * (interaction_distance * interaction_distance - d2s[i][j]);
      affinities[i][j] = std::max(std::min(affinities[i][j], 1.0), 0.0);

      if (affinities[i][j] > 0.8) {
        entities[i]->interact(*entities[j]);
        // std::cout << i << " and " << j << " interacted." << std::endl;

        // Eating.
        bool ieatj =
            entities[i]->current_strength() > entities[j]->current_strength();
        int eater = ieatj ? i : j;
        int target = ieatj ? j : i;

        if (entities[eater]->is_hungry() &&
            entities[eater]->will_eat_target(entities[target])) {
          entities[eater]->consume(*entities[target]);
          std::cout << entities[eater]->name_hash() << " ate "
                    << entities[target]->name_hash() << "!" << std::endl;
          continue;
        }
      }
      if ((entities[i]->gene_value("geodispersal gametophytes/not") &
           entities[j]->gene_value("geodispersal gametophytes/not")) ||
          affinities[i][j] > 0.8) {
        if (entities[i]->will_mate() && entities[j]->will_mate() &&
            entities[i]->will_mate_target(entities[j])) {
          // Mating.
          offspring.push_back(entities[i]->mate(*entities[j]));
          std::cout << entities[i]->name_hash() << " and "
                    << entities[j]->name_hash()
                    << " mated (name: " << offspring.back()->name_hash()
                    << ")." << std::endl;
        }
      }
    }
  }

  entities.insert(entities.end(), std::make_move_iterator(offspring.begin()),
                  std::make_move_iterator(offspring.end()));

  resize_pairwise();

  // Determine which entities we are removing.
  std::vector<int> to_erase;
  for (int i = 0; i < num_entities(); i++) {
    if (!entities[i]->alive_value() &&
        entities[i]->time_since_death() > Entity::corpse_lifetime) {
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
  for (auto &e: entities)
    if (e->will_die_value())
      e->kill();

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

void State::add_entity(const std::string &name, double x, double y,
                       double conception_mass) {
  if (x == 0.0)
    x = newx_dist(*random_generator);
  if (y == 0.0)
    y = newy_dist(*random_generator);
  entities.emplace_back(new Entity(this, name, x, y, conception_mass));

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

std::default_random_engine *State::get_random_generator() const {
  return random_generator;
}

const int State::num_entities() const { return entities.size(); }

const std::vector<Entity *> State::entities_value() const { return entities; }

const std::vector<std::vector<double>> *State::d2s_value() const {
  return &d2s;
}

void State::minimum_vector(const Entity *a, const Entity *b, double &dx,
                           double &dy) const {
  dx = a->x_value() - b->x_value();
  dy = a->y_value() - b->y_value();

  if (abs(dx) > x_size - abs(dx))
    dx = x_size - abs(dx);

  if (abs(dy) > y_size - abs(dy))
    dy = y_size - abs(dy);
}

void State::intecept_trajectory(const Entity *actor, const Entity *target,
                                double travel_time, double &Vhx,
                                double &Vhy) const {
  const double &t = travel_time;
  double Vtx, Vty, Ptx, Pty, Phx, Phy, sh, dx, dy;

  sh = actor->terminal_speed();
  Ptx = target->x_value();
  Pty = target->y_value();
  Vtx = target->px_value();
  Vty = target->py_value();

  // Adjust based on periodic boundaries.
  minimum_vector(actor, target, dx, dy);
  Phx = dx + Ptx;
  Phy = dy + Pty;

  Vhx = (Ptx - Phx + (t * Vtx)) / (t * sh);
  Vhy = (Pty - Phy + (t * Vty)) / (t * sh);
}

double State::entity_intercept_time(const Entity *actor,
                                    const Entity *target) const {
  // Compute intersection.
  double a, b, c, t1, t2, st, dx, dy, Phx, Phy;
  double sh = actor->terminal_speed();
  double Ptx = target->x_value();
  double Pty = target->y_value();
  double Vtx = target->px_value();
  double Vty = target->py_value();

  if (Ptx == 0.0 || Pty == 0.0) {
    std::cout << target << " " << Ptx << " " << Pty << " " << Vtx << " " << Vty
              << std::flush;
    assert(false);
  }

  // Adjust based on periodic boundaries.
  minimum_vector(actor, target, dx, dy);
  Phx = dx + Ptx;
  Phy = dy + Pty;

  st = std::sqrt(Vtx * Vtx + Vty * Vty);

  a = (Vtx * Vtx) + (Vty * Vty) - (sh * sh);
  b = 2 * ((Ptx * Vtx) + (Pty * Vty) - (Phx * Vtx) - (Phy * Vty));
  c = (Ptx * Ptx) + (Pty * Pty) + (Phx * Phx) + (Phy * Phy) - (2 * Phx * Ptx) -
      (2 * Phy * Pty);

  t1 = (-b + sqrt((b * b) - (4 * a * c))) / (2 * a);
  t2 = (-b - sqrt((b * b) - (4 * a * c))) / (2 * a);

  return smallest_non_negative_or_NaN(t1, t2);
}

const Entity *State::nearest_target(Entity *actor, double &time_of_travel,
                                    std::string looking_for) const {
  time_of_travel = std::numeric_limits<double>::infinity();
  const Entity *target = NULL;
  double t;

  for (int i = 0; i < d2s.size(); i++) {
    if (entities[i] != actor)
      continue;
    for (int j = i + 1; j < d2s[i].size(); j++) {
      if (entities[j] == actor)
        continue;
      if (entities[j]->host_value() != NULL)
        continue;

      if (looking_for == "mate") {
        if (!actor->will_mate_target(entities[j]))
          continue;
      } else {
        if (!actor->will_eat_target(entities[j]))
          continue;
      }

      t = entity_intercept_time(actor, entities[j]);

      if (t > 0.0 && t < time_of_travel) {
        time_of_travel = t;
        target = entities[j];
      }
    }
  }

  return target;
}

void State::clear_target_from_entities(Entity *entity) {
  for (auto &e: entities) {
    const Entity *current_target = e->current_target_value();
    if (current_target != NULL && entity == current_target) {
      e->clear_current_target();
    }
  }
}

int State::trait_index(std::string trait) {
  std::vector<std::string>::iterator iter =
      std::find(Entity::all_traits.begin(), Entity::all_traits.end(), trait);
  assert(iter != Entity::all_traits.end());
  return std::distance(Entity::all_traits.begin(), iter);
}

int State::entity_index(const Entity *entity) const {
  for (int i = 0; i < entities.size(); i++) {
    if (entities[i] == entity)
      return i;
  }
  assert(false);
}
