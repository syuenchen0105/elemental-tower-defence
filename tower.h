#ifndef TOWER_H
#define TOWER_H

#include "models.h"

bool place_tower(
    game_data &game,
    int spot_index,
    tower_type type
);

void handle_tower_click(game_data &game);

bool enemy_in_range(
    const tower &current_tower,
    const tower_spot &spot,
    const enemy &current_enemy
);

int find_target(
    const tower &current_tower,
    const tower_spot &spot,
    const dynamic_array<enemy> &enemies,
    const dynamic_array<point_2d> &path
);

void update_tower_attacks(game_data &game, double current_time);

void change_tower_targeting(game_data &game, targeting_mode mode);

int get_upgrade_cost(const game_data &game, const tower &current_tower);

bool upgrade_tower(game_data &game);

#endif