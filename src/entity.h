#ifndef ENTITY_H
#define ENTITY_H

#include <random>

class State;
class Entity {

private:
  State *parent;
  std::string name;
  bool alive, will_die;
  double x, y;
  double px, py;
  double mood, energy, age, birth_mass;
  long epoch_of_death;
  const Entity *current_target;
  std::normal_distribution<double> d;
  std::uniform_real_distribution<double> u;
  std::vector<int> genome;

public:
  static constexpr double year = 86400 * 365;
  static constexpr double min_mood = -5;
  static constexpr double max_mood = 5;
  static constexpr double mate_mood = -1;
  static constexpr double mood_distance = 6;
  static constexpr double mate_energy_coefficient = 0.1;
  static constexpr double eating_energy_coefficient = 0.05;
  static constexpr double hunger_threshold = 0.3;
  static constexpr double birth_mass_coefficient = 0.1;
  static constexpr double kill_energy_coefficient = 10;
  static constexpr double max_energy_coefficient = 10;
  static constexpr double max_speed = 0.2;
  static constexpr double terminal_speed_coefficient = 4;
  static constexpr double mating_age = 86400 * 120;
  static constexpr double mating_distance = 4;
  static constexpr double impotence_age = 20 * year;
  static constexpr double corpse_lifetime = 86400 * 60;
  static constexpr double target_forget_probability = 0.0002;

  static constexpr double always_eat_distance = mating_distance + 3;
  static constexpr double never_eat_distance = mating_distance - 1;

  static std::vector<std::string> all_traits;

  Entity(State *parent, const std::string &, double, double, double);
  ~Entity();
  bool alive_value() const;
  bool will_die_value() const;
  double x_value() const;
  double y_value() const;
  double px_value() const;
  double py_value() const;
  double mood_value() const;
  double age_value() const;
  double energy_value() const;
  double max_speed_value() const;

  long epoch_of_death_value() const;
  long time_since_death() const;
  double kill_energy() const;
  double max_energy() const;
  double prob_wants_food() const;
  double prob_wants_mate() const;
  double prob_mutation() const;
  double prob_death() const;
  double current_strength() const;
  double current_mass() const;
  double terminal_speed() const;
  double mate_energy() const;
  double birth_energy() const;
  double eating_energy() const;
  std::string name_value() const;
  const std::vector<int> *genome_value() const;
  const State *parent_value() const;
  bool can_eat_target(const Entity &) const;
  bool will_eat_target(const Entity &) const;
  bool will_mate_target(const Entity &);
  bool will_mate() const;
  bool is_hungry() const;

  void adjust_mood(double adjustment);
  void adjust_energy(double adjustment);
  void adjust_needs();
  void check_for_death();
  void set_genome(std::vector<int>);
  void consume(Entity &other);
  void kill();
  void clear_current_target();
  int genome_distance(const std::vector<int> *,
                      const std::vector<int> *) const;
  int gene_value(std::string) const;
  void move();
  void interact(Entity &other);
  Entity mate(Entity &other);
};

#endif
