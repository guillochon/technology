#include "entity.h"
#include "state.h"
#include <cstdlib>
#include <random>

std::vector<std::string> Entity::all_traits = {
    "stationary/mobile",        "asexual/sexual",
    "intelligent/passive",      "eats intelligent/not",
    "eats passive/not",         "photosynthesizes/not",
    "geodispersal embryos/not", "geodispersal gametophytes/not",
    "natural defenses/not",     "e"};

Entity::Entity(State *parent, const std::string &name, double x, double y,
               double birth_mass)
    : parent(parent), name(name), alive(true), will_die(false), x(x), y(y),
      px(0), py(0), mood(0), age(0l), birth_mass(birth_mass),
      current_target(NULL) {
  std::default_random_engine *gen = parent->get_gen();

  assert(x != 0.0 && y != 0.0);

  d = std::normal_distribution<double>(0.0, 1.0);
  std::uniform_int_distribution<int> gene(0, 1);
  u = std::uniform_real_distribution<double>(0.0, 1.0);

  genome = std::vector<int>(all_traits.size());

  for (int i = 0; i < genome.size(); i++) {
    genome[i] = gene(*gen);
  }

  adjust_energy(max_energy() * (0.2 + 0.8 * u(*gen)));
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

long Entity::epoch_of_death_value() const { return epoch_of_death; }

long Entity::time_since_death() const {
  return parent->epoch_value() - epoch_of_death;
}

double Entity::kill_energy() const {
  return kill_energy_coefficient * current_mass();
}

double Entity::max_energy() const {
  return birth_mass + current_mass() * max_energy_coefficient * age / year;
}

double Entity::prob_wants_food() const {
  return std::max(0.0, 1.0 - energy / (hunger_threshold * max_energy()));
}

double Entity::prob_wants_mate() const {
  if (!will_mate())
    return 0.0;

  double maxe = max_energy();
  double mate = mate_energy();
  if (maxe < mate)
    return 0.0;
  return std::max(0.0, (energy - mate) / (maxe - mate));
}

double Entity::prob_mutation() const {
  return std::min(1.0, age / impotence_age);
}

double Entity::prob_death() const {
  return std::min(1.0, 0.002 * age / impotence_age);
}

double Entity::current_strength() const {
  return (gene_value("natural defenses/not") ? max_energy() : 0.0) +
         (gene_value("stationary/mobile") ? 0.0 : age / year * energy);
}

double Entity::current_mass() const {
  return birth_mass + std::log(1.0 + age / year);
}

double Entity::terminal_speed() const {
  // Assumes creatures are all same density.
  return terminal_speed_coefficient * pow(current_mass(), 1.0 / 6.0);
}

double Entity::mate_energy() const {
  return mate_energy_coefficient * max_energy() *
         (gene_value("geodispersal gametophytes/not") ? 0.1 : 1.0);
}

double Entity::birth_energy() const { return 2.0 * mate_energy(); }

double Entity::eating_energy() const {
  return eating_energy_coefficient * max_energy();
}

std::string Entity::name_value() const { return name; }

const std::vector<int> *Entity::genome_value() const { return &genome; }

const State *Entity::parent_value() const { return parent; }

bool Entity::will_mate() const {
  return mood > mate_mood && energy >= mate_energy() && age > mating_age &&
         age < impotence_age;
}

bool Entity::is_hungry() const {
  if (!gene_value("intelligent/passive"))
    return false;
  return (energy < hunger_threshold * max_energy());
}

void Entity::adjust_mood(double adjustment) {
  if (1 - gene_value("intelligent/passive")) {
    mood = 0.0;
  } else {
    mood = std::min(max_mood, std::max(min_mood, mood + adjustment));
  }
}

void Entity::adjust_energy(double adjustment) {
  double maxe = max_energy();
  energy = std::min(maxe, std::max(0.0, energy + adjustment));
}

void Entity::adjust_needs() {
  age += parent->tick_time;
  mood *= 0.998;
  adjust_energy(-0.0005 - 0.0005 * gene_value("natural defenses/not") +
                std::min(mood * 0.01, 0.0) +
                0.0012 * gene_value("photosynthesizes/not"));
}

void Entity::check_for_death() {
  if (alive) {
    if (energy <= 0.0) {
      kill();
      std::cout << "Entity " << parent->entity_index(this)
                << " starved to death." << std::endl;
    } else if (u(*parent->get_gen()) < prob_death()) {
      kill();
      std::cout << "Entity " << parent->entity_index(this)
                << " died of natural causes." << std::endl;
    }
  }
}

void Entity::move() {
  if (!alive)
    return;

  if (gene_value("stationary/mobile"))
    return;

  std::default_random_engine *gen = parent->get_gen();

  double ts = terminal_speed();
  double a = std::min(px * px + py * py, ts * ts) / (ts * ts);
  double drag = 1.0 - a;
  double dpx = 0.0, dpy = 0.0, time_of_travel = 0.0;
  double norm, dist;

  std::tuple<double, double> dp;

  px *= drag;
  py *= drag;

  if (energy >= 0.0) {
    int intelligent = gene_value("intelligent/passive");
    if (intelligent) {
      if (current_target != NULL) {
        time_of_travel = parent->entity_intercept_time(this, current_target);
        if (u(*parent->get_gen()) < target_forget_probability)
          current_target = NULL;
      }
      if (current_target == NULL &&
          u(*parent->get_gen()) < prob_wants_food()) {
        // Move to nearest food.
        current_target = parent->nearest_target(this, time_of_travel, "food");
      }
      if (current_target == NULL &&
          u(*parent->get_gen()) < prob_wants_mate()) {
        // Move to nearest mate.
        current_target = parent->nearest_target(this, time_of_travel, "mate");
      }
    }

    if (current_target != NULL && time_of_travel > 0.0) {
      double dx, dy;

      parent->intecept_trajectory(this, current_target, time_of_travel, dpx,
                                  dpy);

      dpx *= max_speed / ts;
      dpy *= max_speed / ts;

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
    adjust_energy(-0.0002 * std::sqrt(abs(px * dpx) + abs(py * dpy)));
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

int Entity::gene_value(std::string trait) const {
  return genome[parent->trait_index(trait)];
}

void Entity::interact(Entity &other) {
  int dist = genome_distance(&genome, other.genome_value());

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

  energy -= mate_energy();
  other.energy -= other.mate_energy();

  double spawn_d = gene_value("geodispersal embryos/not") ? 5 : 250;
  double new_x = 0.5 * (x + other.x) + spawn_d * d(*parent->get_gen());
  double new_y = 0.5 * (y + other.y) + spawn_d * d(*parent->get_gen());
  double offspring_mass =
      birth_mass_coefficient * 0.5 * (current_mass() + other.current_mass());

  if (gene_value("geodispersal gametophytes/not") and
      other.gene_value("geodispersal gametophytes/not")) {
    offspring_mass *= 0.1;
  }

  Entity ret = Entity(parent, new_name, new_x, new_y, offspring_mass);

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

bool Entity::will_mate_target(const Entity &target) {
  if (target.energy_value() < target.mate_energy())
    return false;

  int genetic_diff = genome_distance(&genome, &target.genome);

  if (genetic_diff <= mating_distance) {
    if (gene_value("geodispersal gametophytes/not") &&
        target.gene_value("geodispersal gametophytes/not")) {
      double dist = std::sqrt(pow(x - target.x, 2) + pow(y - target.y, 2));
      if (u(*parent->get_gen()) < 1.0 - pow(1.0 / dist, 2))
        return false;
    }
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

  if (target.energy + target.kill_energy() < eating_energy())
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

void Entity::consume(Entity &target) {
  if (target.will_die)
    return;

  current_target = NULL;

  double de = target.energy + target.kill_energy() - eating_energy();
  adjust_energy(de);
  adjust_mood(target.energy / target.max_energy());
  target.energy = 0;
  target.mood = 0;
  target.px = 0;
  target.py = 0;
  target.will_die = true;
}

void Entity::kill() {
  will_die = false;
  alive = false;
  px = 0.0;
  py = 0.0;
  epoch_of_death = parent->epoch_value();
}

void Entity::clear_current_target() { current_target = NULL; }
