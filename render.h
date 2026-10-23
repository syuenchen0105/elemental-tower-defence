#ifndef RENDER_H
#define RENDER_H

#include "models.h"

void draw_tower_spots(const dynamic_array<tower_spot> &spots);

void draw_towers(
    const dynamic_array<tower> &towers,
    const dynamic_array<tower_spot> &spots
);

void draw_enemies(const dynamic_array<enemy> &enemies);

void draw_projectiles(const dynamic_array<projectile> &projectiles);

void draw_purchase_menu(game_data &game);

void draw_management_menu(game_data &game);

void draw_game(game_data &game);

#endif