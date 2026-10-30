#ifndef ENEMY_H
#define ENEMY_H

#include "models.h"

enemy_type get_wave_enemy_type(
    int wave,
    int spawn_index,
    int enemies_per_wave);

void spawn_enemy(
    game_data &game,
    enemy_type type);

void update_enemies(
    dynamic_array<enemy> &enemies,
    const dynamic_array<point_2d> &path,
    int &lives,
    double current_time);

void remove_inactive_enemies(
    dynamic_array<enemy> &enemies);

#endif