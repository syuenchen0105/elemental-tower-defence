#ifndef TOWER_H
#define TOWER_H

#include "models.h"

// Purchase and place a tower at an available spot
bool place_tower(
    game_data &game,
    int spot_index,
    tower_type type);

// Upgrade the selected tower if sufficient coins are available
bool upgrade_tower(
    game_data &game);

// Get the cost of the next upgrade, or zero if unavailable
int get_upgrade_cost(
    const game_data &game,
    const tower &current_tower);

// Calculate the 70% refund based on the tower's total cost
int get_sell_refund(
    const tower &current_tower);

// Sell the selected tower and make its spot available again
bool sell_tower(
    game_data &game);

// Change the targeting strategy of the selected tower
void change_tower_targeting(
    game_data &game,
    targeting_mode mode);

// Handle mouse clicks to open tower purchase or management menus
void handle_tower_click(
    game_data &game);

// Find valid enemy targets and fire projectiles when towers are ready
void update_tower_attacks(
    game_data &game,
    double current_time);

#endif