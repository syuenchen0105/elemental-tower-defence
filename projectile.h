#ifndef PROJECTILE_H
#define PROJECTILE_H

#include "models.h"

// Create a projectile that travels toward the specified enemy
void fire_projectile(
    dynamic_array<projectile> &projectiles,
    const tower &current_tower,
    const tower_spot &spot,
    int target_enemy_id);

// Move projectiles and process damage, resistances and elemental reactions
void update_projectiles(
    dynamic_array<projectile> &projectiles,
    dynamic_array<enemy> &enemies,
    int &coins,
    double current_time);

// Remove projectiles that have hit or lost their targets
void remove_inactive_projectiles(
    dynamic_array<projectile> &projectiles);

#endif