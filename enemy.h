#ifndef ENEMY_H
#define ENEMY_H

#include "models.h"

void spawn_enemy(game_data &game);

void update_enemies(
    dynamic_array<enemy> &enemies,
    const dynamic_array<point_2d> &path,
    int &lives
);

void remove_inactive_enemies(dynamic_array<enemy> &enemies);

#endif