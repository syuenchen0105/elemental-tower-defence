#ifndef CONFIG_H
#define CONFIG_H

#include "models.h"

// Load the base statistics of all tower types from towers.json
void load_tower_stats(
    dynamic_array<tower_stats> &tower_definitions);

// Find the statistics of a specified tower type
const tower_stats *find_tower_stats(
    const game_config &config,
    tower_type type);

// Load tower upgrade costs and improvements from upgrades.json
void load_tower_upgrades(
    dynamic_array<tower_upgrade> &tower_upgrades);

// Find the upgrade information for a specified tower type and level
const tower_upgrade *find_tower_upgrade(
    const game_config &config,
    tower_type type,
    int level);

// Load the base statistics of all enemy types from enemies.json
void load_enemy_stats(
    dynamic_array<enemy_stats> &enemy_definitions);

// Find the statistics of a specified enemy type
const enemy_stats *find_enemy_stats(
    const game_config &config,
    enemy_type type);

#endif