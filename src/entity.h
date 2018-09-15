#ifndef ENTITY_H
#define ENTITY_H

#include <random>

class State;
class Entity {

private:
  State *parent;
  std::string name;
  bool alive;
  double x, y;
  double px, py;
  double mood, age;
  long epoch_of_death;
  std::normal_distribution<double> d;
  std::vector<int> genome;

public:
  Entity(State *parent, const std::string &name, double x, double y);
  bool alive_value() const;
  double x_value() const;
  double y_value() const;
  double px_value() const;
  double py_value() const;
  double mood_value() const;
  double age_value() const;
  long epoch_of_death_value() const;
  long time_since_death() const;
  std::string name_value() const;
  const std::vector<int> *genome_value() const;
  const State *parent_value() const;
  void adjust_mood(double adjustment);
  void adjust_needs();
  void check_for_death();
  int genome_distance(const std::vector<int> *a,
                      const std::vector<int> *b) const;
  void move();
  void interact(Entity &other);
  Entity mate(Entity &other);
};

#endif
