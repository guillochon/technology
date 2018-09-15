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
  double mood, energy, age;
  long epoch_of_death;
  std::normal_distribution<double> d;
  std::vector<int> genome;

public:
  static constexpr double min_mood = -5;
  static constexpr double max_mood = 5;
  static constexpr double mate_mood = 3;
  static constexpr double mate_energy = 3;
  static constexpr double hunger_threshold = 5;
  static constexpr double kill_energy = 50;
  static constexpr double max_speed = 0.1;

  Entity(State *parent, const std::string &, double, double);
  bool alive_value() const;
  bool will_die_value() const;
  double x_value() const;
  double y_value() const;
  double px_value() const;
  double py_value() const;
  double mood_value() const;
  double age_value() const;
  double energy_value() const;
  long epoch_of_death_value() const;
  long time_since_death() const;
  std::string name_value() const;
  const std::vector<int> *genome_value() const;
  const State *parent_value() const;
  bool can_mate(const Entity &other) const;
  bool can_eat(const Entity &other) const;
  bool will_mate() const;
  bool is_hungry() const;

  void adjust_mood(double adjustment);
  void adjust_needs();
  void check_for_death();
  void set_genome(std::vector<int>);
  void consume(Entity &other);
  void kill();
  int genome_distance(const std::vector<int> *,
                      const std::vector<int> *) const;
  void move();
  void interact(Entity &other);
  Entity mate(Entity &other);
};

#endif
