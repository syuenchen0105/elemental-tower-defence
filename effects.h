#ifndef EFFECTS_H
#define EFFECTS_H

#include "models.h"

void apply_effect(
    enemy &current_enemy,
    effect_type type,
    double current_time,
    double duration);

bool is_effect_active(
    const enemy &current_enemy,
    effect_type type,
    double current_time);

void apply_wet_slow(
    enemy &current_enemy,
    double current_time,
    double duration);

void damage_enemy(
    enemy &current_enemy,
    int damage,
    int &coins);

void update_enemy_effects(
    dynamic_array<enemy> &enemies,
    double current_time,
    int &coins);

bool can_trigger_reaction(
    const enemy &current_enemy,
    double current_time);

void start_reaction_cooldown(
    enemy &current_enemy,
    double current_time,
    double duration);

void try_trigger_reaction(
    dynamic_array<enemy> &enemies,
    int enemy_index,
    double current_time,
    int &coins);

#endif