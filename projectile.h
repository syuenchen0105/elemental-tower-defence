#ifndef PROJECTILE_H
#define PROJECTILE_H

#include "models.h"

void fire_projectile(
    dynamic_array<projectile> &projectiles,
    const tower &current_tower,
    const tower_spot &spot,
    int target_enemy_id
);

void update_projectiles(
    dynamic_array<projectile> &projectiles,
    dynamic_array<enemy> &enemies,
    int &coins,
    double current_time
);

void remove_inactive_projectiles(dynamic_array<projectile> &projectiles);

#endif