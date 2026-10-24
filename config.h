#ifndef CONFIG_H
#define CONFIG_H

#include "models.h"

tower_stats read_tower_stats(
    json tower_data,
    const string &type_name,
    tower_type type
);

tower_upgrade read_tower_upgrade(
    json upgrade_data,
    const string &type_name,
    tower_type type,
    int level
);

void load_tower_stats(dynamic_array<tower_stats> &tower_definitions);

void load_tower_upgrades(
    dynamic_array<tower_upgrade> &tower_upgrades
);

const tower_stats *find_tower_stats(
    const game_config &config,
    tower_type type
);

const tower_upgrade *find_tower_upgrade(
    const game_config &config,
    tower_type type,
    int level
);

#endif