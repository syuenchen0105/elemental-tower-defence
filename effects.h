#ifndef EFFECTS_H
#define EFFECTS_H

#include "models.h"

// Apply or extend an elemental status effect on an enemy
void apply_effect(
    enemy &current_enemy,
    effect_type type,
    double current_time,
    double duration);

// Check whether an enemy currently has a specified elemental status
bool is_effect_active(
    const enemy &current_enemy,
    effect_type type,
    double current_time);

// Apply or extend the movement slowdown caused by a WATER tower
void apply_wet_slow(
    enemy &current_enemy,
    double current_time,
    double duration);

// Reduce enemy health and award coins when the enemy is defeated
void damage_enemy(
    enemy &current_enemy,
    int damage,
    int &coins);

// Update ongoing elemental effects such as periodic BURNING damage
void update_enemy_effects(
    dynamic_array<enemy> &enemies,
    double current_time,
    int &coins);

// Check whether an enemy's elemental reaction cooldown has expired
bool can_trigger_reaction(
    const enemy &current_enemy,
    double current_time);

// Start a cooldown after an elemental reaction is triggered
void start_reaction_cooldown(
    enemy &current_enemy,
    double current_time,
    double duration);

// Detect and apply elemental reactions between two active statuses
void try_trigger_reaction(
    dynamic_array<enemy> &enemies,
    int enemy_index,
    double current_time,
    int &coins);

#endif