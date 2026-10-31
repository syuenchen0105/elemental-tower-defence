#ifndef RENDER_H
#define RENDER_H

#include "models.h"

// Draw empty tower placement spots on the map
void draw_tower_spots(
    const dynamic_array<tower_spot> &spots);

// Draw purchased towers using their elemental colours
void draw_towers(
    const dynamic_array<tower> &towers,
    const dynamic_array<tower_spot> &spots);

// Draw active enemies with different colours, sizes and health bars
void draw_enemies(
    const dynamic_array<enemy> &enemies);

// Draw all active projectiles
void draw_projectiles(
    const dynamic_array<projectile> &projectiles);

// Display the tower purchase menu and handle tower purchases
void draw_purchase_menu(
    game_data &game);

// Display tower targeting, upgrade and selling options
void draw_management_menu(
    game_data &game);

// Draw the complete game screen, HUD, menus and preparation panel
void draw_game(
    game_data &game);

#endif