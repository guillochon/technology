#include "entity.h"
#include "state.h"
#include <cstdlib>
#include <random>

Entity::Entity(State *parent, const std::string &name, double x, double y)
    : parent(parent), name(name), alive(true), will_die(false), x(x), y(y),
      px(0), py(0), mood(0), age(0l), current_target(NULL) {
  std::default_random_engine *gen = parent->get_gen();

  assert(x != 0.0 && y != 0.0);

  d = std::normal_distribution<double>(0.0, 1.0);
  std::uniform_int_distribution<int> gene(0, 1);
  u = std::uniform_real_distribution<double>(0.0, 1.0);

  energy = birth_energy + birth_energy * u(*gen);

  genome = std::vector<int>(10);

  for (int i = 0; i < genome.size(); i++) {
    genome[i] = gene(*gen);
  }
}

Entity::~Entity() {
  for (int i = 0; i < parent->entities_value()->size(); i++) {
    const Entity *current_target =
        parent->entities_value()->at(i).current_target;
    if (current_target != NULL && this == current_target) {
      parent->clear_entity_target(i);
    }
  }
}

bool Entity::alive_value() const { return alive; }

bool Entity::will_die_value() const { return will_die; }

double Entity::x_value() const { return x; }

double Entity::y_value() const { return y; }

double Entity::px_value() const { return px; }

double Entity::py_value() const { return py; }

double Entity::mood_value() const { return mood; }

double Entity::age_value() const { return age; }

double Entity::energy_value() const { return energy; }

double Entity::max_speed_value() const { return max_speed; }

double Entity::terminal_speed_value() const { return terminal_speed; }

long Entity::epoch_of_death_value() const { return epoch_of_death; }

long Entity::time_since_death() const {
  return parent->epoch_value() - epoch_of_death;
}

double Entity::kill_energy() const {
  return kill_energy_coefficient * age / year;
}

double Entity::max_energy() const {
  return std::max(birth_energy, max_energy_coefficient * age / year);
}

double Entity::prob_wants_food() const {
  return std::max(0.0, 1.0 - energy / (hunger_threshold * max_energy()));
}

double Entity::prob_wants_mate() const {
  if (!will_mate())
    return 0.0;

  double maxe = max_energy();
  if (maxe < mate_energy)
    return 0.0;
  return std::max(0.0, (energy - mate_energy) / (maxe - mate_energy));
}

double Entity::prob_mutation() const {
  return std::min(1.0, age / impotence_age);
}

double Entity::current_strength() const { return age / year * energy; }

std::string Entity::name_value() const { return name; }

const std::vector<int> *Entity::genome_value() const { return &genome; }

const State *Entity::parent_value() const { return parent; }

bool Entity::will_mate() const {
  return mood > mate_mood && energy >= mate_energy && age > mating_age &&
         age < impotence_age;
}

bool Entity::is_hungry() const {
  return energy < hunger_threshold * max_energy();
}

void Entity::adjust_mood(double adjustment) {
  mood = std::min(max_mood, std::max(min_mood, mood + adjustment));
}

void Entity::adjust_energy(double adjustment) {
  double maxe = max_energy();
  energy = std::min(maxe, std::max(0.0, energy + adjustment));
}

void Entity::adjust_needs() {
  age += parent->tick_time;
  mood *= 0.998;
  adjust_energy(0.01 + std::min(mood * 0.01, 0.0));
}

void Entity::check_for_death() {
  if (alive && energy <= 0.0)
    kill();
}

void Entity::move() {
  if (!alive)
    return;

  std::default_random_engine *gen = parent->get_gen();

  double a = std::min(px * px + py * py, terminal_speed * terminal_speed) /
             (terminal_speed * terminal_speed);
  double drag = 1.0 - a;
  double dpx = 0.0, dpy = 0.0, time_of_travel = 0.0;
  double norm, dist;

  std::tuple<double, double> dp;

  px *= drag;
  py *= drag;

  if (energy >= 0.0) {
    if (current_target != NULL) {
      time_of_travel = parent->entity_intercept_time(this, current_target);
    }
    if (u(*parent->get_gen()) < target_forget_probability)
      current_target = NULL;
    if (current_target == NULL && u(*parent->get_gen()) < prob_wants_food()) {
      // Move to nearest food.
      current_target = parent->nearest_target(this, time_of_travel, "food");
    }
    if (current_target == NULL && u(*parent->get_gen()) < prob_wants_mate()) {
      // Move to nearest mate.
      current_target = parent->nearest_target(this, time_of_travel, "mate");
    }

    if (current_target != NULL && time_of_travel > 0.0) {
      double dx, dy;

      parent->intecept_trajectory(this, current_target, time_of_travel, dpx,
                                  dpy);

      norm = std::sqrt(dpx * dpx + dpy * dpy);

      parent->minimum_vector(this, current_target, dx, dy);
      dist = std::sqrt(pow(dx, 2) + pow(dy, 2));
      dpx /= std::max(norm / dist, 1.0);
      dpy /= std::max(norm / dist, 1.0);

      // std::cout << "tot:" << time_of_travel << ", dpx: " << dpx
      //           << ", dpy: " << dpy << std::endl;

      if (std::isnan(dpx) || std::isnan(dpy)) {
        std::cout << x << " " << y << " " << current_target->x << " "
                  << current_target->y << std::endl;
        assert(false);
      }

      // Add a little jitter.
      dpx += 0.2 * max_speed * d(*gen);
      dpy += 0.2 * max_speed * d(*gen);
    } else {
      // Random walk.
      dpx = max_speed * d(*gen);
      dpy = max_speed * d(*gen);
    }
    px += dpx;
    py += dpy;
    energy = energy - 0.02 * std::sqrt(dpx * dpx + dpy * dpy);
  }

  x += px;
  if (x > parent->x_size_value()) {
    x -= parent->x_size_value();
  } else if (x < 0) {
    x += parent->x_size_value();
  }

  y += py;
  if (y > parent->y_size_value()) {
    y -= parent->y_size_value();
  } else if (y < 0) {
    y += parent->y_size_value();
  }
}

int Entity::genome_distance(const std::vector<int> *a,
                            const std::vector<int> *b) const {
  assert(a->size() == b->size());

  int dist = 0;
  for (int i = 0; i < a->size(); i++) {
    dist += abs(a->at(i) - b->at(i));
  }

  return dist;
}

void Entity::interact(Entity &other) {
  int dist = genome_distance(&genome, other.genome_value());

  // std::cout << "Distance: " << dist << std::endl;

  if (dist <= mood_distance) {
    adjust_mood(1);
    other.adjust_mood(1);
  } else {
    adjust_mood(-1);
    other.adjust_mood(-1);
  }

  // std::cout << "Moods are now " << mood_value() << " and "
  //           << other.mood_value() << std::endl;
}

void Entity::set_genome(std::vector<int> new_genome) { genome = new_genome; }

Entity Entity::mate(Entity &other) {
  std::uniform_int_distribution<int> gene(0, 1);
  std::string new_name = "(" + name + " + " + other.name + ")";
  adjust_mood(-1);
  other.adjust_mood(-1);

  energy -= mate_energy;
  other.energy -= mate_energy;

  Entity ret =
      Entity(parent, new_name, 0.5 * (x + other.x), 0.5 * (y + other.y));

  std::vector<int> new_genome = genome;
  for (int i = 0; i < genome.size(); i++) {
    if (gene(*parent->get_gen()) == 1) {
      new_genome[i] = other.genome[i];
      if (u(*parent->get_gen()) < other.prob_mutation())
        new_genome[i] = 1 - new_genome[i];
    }
    if (u(*parent->get_gen()) < prob_mutation())
      new_genome[i] = 1 - new_genome[i];
  }
  ret.set_genome(new_genome);

  return ret;
}

bool Entity::will_mate_target(const Entity &target) const {
  int dist = genome_distance(&genome, &target.genome);

  if (dist <= mating_distance) {
    return true;
  } else {
    return false;
  }
}

bool Entity::can_eat_target(const Entity &target) const {
  return target.energy > 0 && target.alive;
}

bool Entity::will_eat_target(const Entity &target) const {
  if (target.energy <= 0 || !target.alive ||
      current_strength() < target.current_strength())
    return false;

  int dist = genome_distance(&genome, &target.genome);

  if (dist > never_eat_distance &&
      dist >= always_eat_distance *
                  std::min(energy / (hunger_threshold * max_energy()), 1.0)) {
    return true;
  } else {
    return false;
  }
}

void Entity::consume(Entity &other) {
  if (other.will_die)
    return;

  if (current_target == &other)
    current_target = NULL;

  double de = other.energy + other.kill_energy();
  adjust_energy(de);
  adjust_mood(other.energy / other.max_energy());
  other.energy = 0;
  other.mood = 0;
  other.px = 0;
  other.py = 0;
  other.will_die = true;
}

void Entity::kill() {
  will_die = false;
  alive = false;
  px = 0.0;
  py = 0.0;
  epoch_of_death = parent->epoch_value();
}

void Entity::clear_current_target() { current_target = NULL; }
