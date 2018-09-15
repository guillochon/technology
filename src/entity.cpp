#include "entity.h"
#include "state.h"
#include <cstdlib>
#include <random>

Entity::Entity(State *parent, const std::string &name, double x, double y)
    : parent(parent), name(name), alive(true), will_die(false), x(x), y(y),
      px(0), py(0), mood(0), energy(10), age(0l) {
  d = std::normal_distribution<double>(0.0, 0.1);
  std::default_random_engine *gen = parent->get_gen();
  std::uniform_int_distribution<int> gene(0, 1);

  genome = std::vector<int>(10);

  for (int i = 0; i < genome.size(); i++) {
    genome[i] = gene(*gen);
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

long Entity::epoch_of_death_value() const { return epoch_of_death; }

long Entity::time_since_death() const {
  return parent->epoch_value() - epoch_of_death;
}

std::string Entity::name_value() const { return name; }

const std::vector<int> *Entity::genome_value() const { return &genome; }

const State *Entity::parent_value() const { return parent; }

bool Entity::will_mate() const {
  return mood > mate_mood && energy >= mate_energy;
}

bool Entity::is_hungry() const { return energy < hunger_threshold; }

void Entity::adjust_mood(double adjustment) {
  mood = std::min(max_mood, std::max(min_mood, mood + adjustment));
}

void Entity::adjust_needs() {
  age += 86400;
  mood *= 0.998;
  energy -= 0.01 + std::max(mood * 0.01, 0.0);
}

void Entity::check_for_death() {
  if (alive && energy <= 0.0) {
    alive = false;
    px = 0;
    py = 0;
    epoch_of_death = parent->epoch_value();
  }
}

void Entity::move() {
  if (!alive)
    return;

  std::default_random_engine *gen = parent->get_gen();

  double a = std::min(px * px + py * py, 100.0) / 100.0;
  double drag = 1.0 - a;

  px *= drag;
  py *= drag;

  if (energy >= 0.0) {
    double dpx = d(*gen), dpy = d(*gen);
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

  std::cout << "Distance: " << dist << std::endl;

  if (dist <= 5) {
    adjust_mood(1);
    other.adjust_mood(1);
  } else {
    adjust_mood(-1);
    other.adjust_mood(-1);
  }

  std::cout << "Moods are now " << mood_value() << " and " << other.mood_value()
            << std::endl;
}

void Entity::set_genome(std::vector<int> new_genome) { genome = new_genome; }

Entity Entity::mate(Entity &other) {
  std::uniform_int_distribution<int> gene(0, 1);
  std::string new_name = name + " + " + other.name;
  adjust_mood(-1);
  other.adjust_mood(-1);

  Entity ret =
      Entity(parent, new_name, 0.5 * (x + other.x), 0.5 * (y + other.y));

  std::vector<int> new_genome = genome;
  for (int i = 0; i < genome.size(); i++) {
    if (gene(*parent->get_gen()) == 1) {
      new_genome[i] = other.genome[i];
    }
  }
  ret.set_genome(new_genome);

  return ret;
}

bool Entity::can_mate(const Entity &other) const {
  int dist = genome_distance(&genome, &other.genome);

  if (dist <= 3) {
    return true;
  } else {
    return false;
  }
}

bool Entity::can_eat(const Entity &other) const {
  int dist = genome_distance(&genome, &other.genome);

  if (dist >= 7) {
    return true;
  } else {
    return false;
  }
}

void Entity::consume(Entity &other) {
  if (other.will_die) return;
  energy += other.energy + kill_energy;
  other.energy = 0;
  other.mood = 0;
  other.px = 0;
  other.py = 0;
  will_die = true;
}

void Entity::kill() {
  alive = false;
}
