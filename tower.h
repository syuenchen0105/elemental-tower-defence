#ifndef TOWER_H
#define TOWER_H

#include "models.h"

bool place_tower(
    game_data &game,
    int spot_index,
    tower_type type);

bool upgrade_tower(game_data &game);

int get_upgrade_cost(
    const game_data &game,
    const tower &current_tower);

int get_sell_refund(
    const tower &current_tower);

bool sell_tower(game_data &game);

void change_tower_targeting(
    game_data &game,
    targeting_mode mode);

void handle_tower_click(game_data &game);

void update_tower_attacks(
    game_data &game,
    double current_time);

#endif