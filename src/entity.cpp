#include "entity.h"
#include "state.h"
#include <cstdlib>
#include <random>

Entity::Entity(State *parent, const std::string &name, double x, double y)
    : parent(parent), name(name), alive(true), x(x), y(y), px(0), py(0),
      mood(0), age(0l) {
  d = std::normal_distribution<double>(0.0, 0.1);
  std::default_random_engine *gen = parent->get_gen();
  std::uniform_int_distribution<int> gene(0, 1);

  genome = std::vector<int>(10, 1);

  for (int i = 0; i < genome.size(); i++) {
    genome[i] = gene(*gen);
  }
}

bool Entity::alive_value() const { return alive; }

double Entity::x_value() const { return x; }

double Entity::y_value() const { return y; }

double Entity::px_value() const { return px; }

double Entity::py_value() const { return py; }

double Entity::mood_value() const { return mood; }

double Entity::age_value() const { return age; }

long Entity::epoch_of_death_value() const { return epoch_of_death; }

long Entity::time_since_death() const {
  return parent->epoch_value() - epoch_of_death;
}

std::string Entity::name_value() const { return name; }

const std::vector<int> *Entity::genome_value() const { return &genome; }

const State *Entity::parent_value() const { return parent; }

void Entity::adjust_mood(double adjustment) { mood += adjustment; }

void Entity::adjust_needs() {
  age += 86400;
  mood *= 0.998;
}

void Entity::check_for_death() {
  if (alive && mood < -3.0) {
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

  double dx = d(*gen);
  px += dx;
  x += px;
  if (x > parent->x_size_value()) {
    x -= parent->x_size_value();
  } else if (x < 0) {
    x += parent->x_size_value();
  }

  double dy = d(*gen);
  py += dy;
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

Entity Entity::mate(Entity &other) {
  std::string new_name = name + " + " + other.name_value();
  adjust_mood(-1);
  other.adjust_mood(-1);
  return Entity(parent, new_name, 0.5 * (x + other.x_value()),
                0.5 * (y + other.y_value()));
}
