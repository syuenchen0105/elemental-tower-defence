#ifndef ENEMY_H
#define ENEMY_H

#include "models.h"

// Determine which enemy type should appear at a given position in a wave
enemy_type get_wave_enemy_type(
    int wave,
    int spawn_index,
    int enemies_per_wave);

// Create an enemy using its configured statistics and current wave scaling
void spawn_enemy(
    game_data &game,
    enemy_type type);

// Move active enemies along the path and reduce lives when enemies escape
void update_enemies(
    dynamic_array<enemy> &enemies,
    const dynamic_array<point_2d> &path,
    int &lives,
    double current_time);

// Remove defeated enemies and enemies that have reached the end of the path
void remove_inactive_enemies(
    dynamic_array<enemy> &enemies);

#endif