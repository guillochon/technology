#ifndef ENTITY_H
#define ENTITY_H

#include <random>

class State;
class Entity {

private:
  State *parent;
  std::string name;
  bool alive, will_die;
  Entity *host;
  std::vector<Entity *> parasites;
  double x, y;
  double px, py;
  double mood, energy, age, conception_mass;
  long epoch_of_death;
  const Entity *current_target;
  std::default_random_engine *random_generator;
  mutable std::normal_distribution<double> d;
  mutable std::uniform_int_distribution<int> gene;
  mutable std::uniform_real_distribution<double> u;
  std::vector<unsigned short> genome;

public:
  static constexpr double year = 86400 * 365;
  static constexpr double min_mood = -5;
  static constexpr double max_mood = 5;
  static constexpr double mate_mood = -1;
  static constexpr double mood_distance = 6;
  static constexpr double mate_energy_coefficient = 0.1;
  static constexpr double eating_energy_coefficient = 0.05;
  static constexpr double hunger_threshold = 0.3;
  static constexpr double conception_mass_coefficient = 0.1;
  static constexpr double kill_energy_coefficient = 0;
  static constexpr double max_energy_coefficient = 10;
  static constexpr double max_speed = 0.2;
  static constexpr double terminal_speed_coefficient = 4;
  static constexpr double mating_age = 86400 * 120;
  static constexpr double mating_distance = 4;
  static constexpr double impotence_age_coefficient = 20 * year;
  static constexpr double birth_age_coefficient = 0.1;
  static constexpr double corpse_lifetime = 86400 * 60;
  static constexpr double target_forget_probability = 0.0002;
  static constexpr double death_probability_coefficient = 0.001;

  static constexpr double always_eat_distance = mating_distance + 3;
  static constexpr double never_eat_distance = mating_distance - 1;

  static std::vector<std::string> all_traits;

  Entity(State *parent, const std::string &, double, double, double,
         std::vector<unsigned short> = {}, Entity * = NULL);
  ~Entity();
  bool alive_value() const;
  bool will_die_value() const;
  Entity *host_value() const;
  double x_value() const;
  double y_value() const;
  double px_value() const;
  double py_value() const;
  double mood_value() const;
  double age_value() const;
  double energy_value() const;
  double max_speed_value() const;
  std::string name_value() const;
  std::string name_hash() const;
  const std::vector<unsigned short> *genome_value() const;
  const State *parent_value() const;
  const Entity *current_target_value() const;

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
  double conception_energy() const;
  double eating_energy() const;
  long impotence_age() const;
  long birth_age() const;
  long age_since_birth() const;
  bool will_mate() const;
  bool is_hungry() const;
  int gene_value(std::string) const;
  int parasite_count() const;

  bool can_eat_target(const Entity *) const;
  bool will_eat_target(const Entity *) const;
  bool will_mate_target(const Entity *) const;

  int genome_distance(const std::vector<unsigned short> *,
                      const std::vector<unsigned short> *) const;

  void adjust_mood(double adjustment);
  void adjust_energy(double adjustment);
  void adjust_needs();
  void check_for_death();
  void set_genome(std::vector<unsigned short>);
  void consume(Entity &other);
  void kill(bool = true);
  void clear_current_target();
  void move();
  void interact(Entity &other);
  Entity *mate(Entity &other);
  void assign_host(Entity *);
};

#endif
