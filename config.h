#ifndef CONFIG_H
#define CONFIG_H

#include "models.h"

void load_tower_stats(
    dynamic_array<tower_stats> &tower_definitions);

const tower_stats *find_tower_stats(
    const game_config &config,
    tower_type type);

void load_tower_upgrades(
    dynamic_array<tower_upgrade> &tower_upgrades);

const tower_upgrade *find_tower_upgrade(
    const game_config &config,
    tower_type type,
    int level);

void load_enemy_stats(
    dynamic_array<enemy_stats> &enemy_definitions);

const enemy_stats *find_enemy_stats(
    const game_config &config,
    enemy_type type);

#endif