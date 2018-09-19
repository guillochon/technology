#ifndef STATE_H
#define STATE_H

#include <iostream>
#include <list>
#include <random>

class Entity;
class State {

private:
  double money;
  long epoch;
  double x_size, y_size;
  std::vector<Entity *> entities;
  std::default_random_engine *random_generator;
  std::vector<std::vector<double>> d2s, affinities;
  std::uniform_real_distribution<double> newx_dist, newy_dist;

public:
  static constexpr double tick_time = 86400;
  static constexpr double interaction_distance = 5.0;

  State(double, long, double, double);
  ~State();
  double money_value() const;
  long epoch_value() const;
  std::string date_str() const;
  std::default_random_engine * get_random_generator() const;
  void update();
  double smallest_non_negative_or_NaN(double, double) const;
  void add_entity(const std::string &, double = 0.0, double = 0.0,
                  double = 1.0);
  void resize_pairwise();
  double x_size_value() const;
  double y_size_value() const;
  const int num_entities() const;
  const std::vector<Entity *> entities_value() const;
  const std::vector<std::vector<double>> *d2s_value() const;
  void minimum_vector(const Entity *, const Entity *, double &,
                      double &) const;
  void intecept_trajectory(const Entity *, const Entity *, double, double &,
                           double &) const;
  double entity_intercept_time(const Entity *, const Entity *) const;
  const Entity *nearest_target(Entity *, double &, std::string) const;
  void clear_targets(Entity *);
  int trait_index(std::string);
  int entity_index(const Entity *entity) const;
};

#endif
