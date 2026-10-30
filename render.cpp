#include "config.h"
#include "render.h"
#include "tower.h"

void draw_tower_spots(const dynamic_array<tower_spot> &spots)
{
    // Draw available tower spots
    for (int i = 0; i < length(spots); i++)
    {
        if (!spots[i].occupied)
        {
            fill_circle(COLOR_BLUE, spots[i].position, 20);
        }
    }
}

void draw_towers(
    const dynamic_array<tower> &towers,
    const dynamic_array<tower_spot> &spots)
{
    // Draw each tower at its corresponding spot
    for (int i = 0; i < length(towers); i++)
    {
        for (int j = 0; j < length(spots); j++)
        {
            if (towers[i].spot_id == spots[j].id)
            {
                color tower_color = COLOR_WHITE;

                switch (towers[i].type)
                {
                case FIRE:
                    tower_color = COLOR_ORANGE;
                    break;

                case WATER:
                    tower_color = COLOR_BLUE;
                    break;

                case ELECTRIC:
                    tower_color = COLOR_YELLOW;
                    break;

                case EARTH:
                    tower_color = COLOR_BROWN;
                    break;
                }

                fill_circle(tower_color, spots[j].position, 15);

                break;
            }
        }
    }
}

void draw_enemies(const dynamic_array<enemy> &enemies)
{
    // Draw all enemies currently in the array
    for (int i = 0; i < length(enemies); i++)
    {
        fill_circle(COLOR_RED, enemies[i].position, 12);
    }
}

void draw_projectiles(const dynamic_array<projectile> &projectiles)
{
    for (int i = 0; i < length(projectiles); i++)
    {
        fill_circle(COLOR_YELLOW, projectiles[i].position, 5);
    }
}

static void select_tower(game_data &game, tower_type type)
{
    if (place_tower(game, game.selected_spot_index, type))
    {
        game.purchase_menu_open = false;
        game.selected_spot_index = -1;
    }
}

static void select_targeting_mode(game_data &game, targeting_mode mode)
{
    change_tower_targeting(game, mode);
    game.management_menu_open = false;
    game.selected_spot_index = -1;
}

static void select_upgrade(game_data &game)
{
    if (upgrade_tower(game))
    {
        game.management_menu_open = false;
        game.selected_spot_index = -1;
    }
}

static void select_sell(game_data &game)
{
    if (sell_tower(game))
    {
        game.management_menu_open = false;
        game.selected_spot_index = -1;
    }
}

static void draw_purchase_option(
    game_data &game,
    tower_type type,
    const string &name,
    rectangle button_area)
{
    const tower_stats *stats = find_tower_stats(game.config, type);

    if (stats == nullptr)
        return;

    if (button(name + " - " + to_string(stats->cost), button_area))
    {
        select_tower(game, type);
    }
}

void draw_purchase_menu(game_data &game)
{
    point_2d spot_position =
        game.tower_spots[game.selected_spot_index].position;

    double menu_width = 180;
    double menu_height = 240;
    double gap = 30;

    double menu_x = spot_position.x + gap;

    if (menu_x + menu_width > 1000)
    {
        menu_x = spot_position.x - gap - menu_width;
    }

    double menu_y = spot_position.y - menu_height / 2;

    if (menu_y < 0)
    {
        menu_y = 0;
    }

    if (menu_y + menu_height > 700)
    {
        menu_y = 700 - menu_height;
    }

    // Draw menu background
    fill_rectangle(
        COLOR_WHITE,
        menu_x,
        menu_y,
        menu_width,
        menu_height);

    draw_rectangle(
        COLOR_BLACK,
        menu_x,
        menu_y,
        menu_width,
        menu_height);

    draw_text(
        "Choose Tower",
        COLOR_BLACK,
        menu_x + 15,
        menu_y + 15);

    double button_width = menu_width - 30;
    double button_height = 30;
    double button_x = menu_x + (menu_width - button_width) / 2 - 5;

    // Draw elemental tower purchase options
    draw_purchase_option(
        game,
        FIRE,
        "FIRE",
        rectangle_from(
            button_x,
            menu_y + 30,
            button_width,
            button_height));

    draw_purchase_option(
        game,
        WATER,
        "WATER",
        rectangle_from(
            button_x,
            menu_y + 70,
            button_width,
            button_height));

    draw_purchase_option(
        game,
        ELECTRIC,
        "ELECTRIC",
        rectangle_from(
            button_x,
            menu_y + 110,
            button_width,
            button_height));

    draw_purchase_option(
        game,
        EARTH,
        "EARTH",
        rectangle_from(
            button_x,
            menu_y + 150,
            button_width,
            button_height));

    if (button(
            "CANCEL",
            rectangle_from(
                button_x,
                menu_y + 190,
                button_width,
                button_height)))
    {
        game.purchase_menu_open = false;
        game.selected_spot_index = -1;
    }
}

void draw_management_menu(game_data &game)
{
    int selected_tower_id =
        game.tower_spots[game.selected_spot_index].tower_id;

    int upgrade_cost = 0;
    int sell_refund = 0;
    int tower_level = 0;

    for (int i = 0; i < length(game.towers); i++)
    {
        if (game.towers[i].id == selected_tower_id)
        {
            upgrade_cost = get_upgrade_cost(game, game.towers[i]);
            sell_refund = get_sell_refund(game.towers[i]);
            tower_level = game.towers[i].level;
            break;
        }
    }

    if (tower_level == 0)
        return;

    point_2d spot_position =
        game.tower_spots[game.selected_spot_index].position;

    double menu_width = 180;
    double menu_height = 320;
    double gap = 30;

    double menu_x = spot_position.x + gap;

    if (menu_x + menu_width > 1000)
    {
        menu_x = spot_position.x - gap - menu_width;
    }

    double menu_y = spot_position.y - menu_height / 2;

    if (menu_y < 0)
    {
        menu_y = 0;
    }

    if (menu_y + menu_height > 700)
    {
        menu_y = 700 - menu_height;
    }

    // Draw menu background
    fill_rectangle(
        COLOR_WHITE,
        menu_x,
        menu_y,
        menu_width,
        menu_height);

    draw_rectangle(
        COLOR_BLACK,
        menu_x,
        menu_y,
        menu_width,
        menu_height);

    draw_text(
        "Tower - Lv " + to_string(tower_level),
        COLOR_BLACK,
        menu_x + 15,
        menu_y + 15);

    double button_width = menu_width - 30;
    double button_height = 30;
    double button_x = menu_x + (menu_width - button_width) / 2 - 5;

    // Draw targeting mode buttons
    if (button(
            "FIRST",
            rectangle_from(
                button_x,
                menu_y + 30,
                button_width,
                button_height)))
    {
        select_targeting_mode(game, FIRST);
    }

    if (button(
            "CLOSEST",
            rectangle_from(
                button_x,
                menu_y + 70,
                button_width,
                button_height)))
    {
        select_targeting_mode(game, CLOSEST);
    }

    if (button(
            "STRONGEST",
            rectangle_from(
                button_x,
                menu_y + 110,
                button_width,
                button_height)))
    {
        select_targeting_mode(game, STRONGEST);
    }

    if (button(
            "WEAKEST",
            rectangle_from(
                button_x,
                menu_y + 150,
                button_width,
                button_height)))
    {
        select_targeting_mode(game, WEAKEST);
    }

    // Draw upgrade button
    string upgrade_text = "MAX LEVEL";

    if (upgrade_cost > 0)
    {
        upgrade_text = "UPGRADE - " + to_string(upgrade_cost);
    }

    if (button(
            upgrade_text,
            rectangle_from(
                button_x,
                menu_y + 190,
                button_width,
                button_height)))
    {
        if (upgrade_cost > 0)
        {
            select_upgrade(game);
        }
    }

    // Draw sell button
    string sell_text = "SELL - " + to_string(sell_refund);

    if (button(
            sell_text,
            rectangle_from(
                button_x,
                menu_y + 230,
                button_width,
                button_height)))
    {
        select_sell(game);
    }

    if (button(
            "CANCEL",
            rectangle_from(
                button_x,
                menu_y + 270,
                button_width,
                button_height)))
    {
        game.management_menu_open = false;
        game.selected_spot_index = -1;
    }
}

void draw_game(game_data &game)
{
    clear_screen(COLOR_WHITE);

    // Draw game information
    draw_text("Tower Defence", COLOR_BLACK, 20, 20);
    draw_text("Coins: " + to_string(game.coins), COLOR_BLACK, 20, 40);
    draw_text("Lives: " + to_string(game.lives), COLOR_BLACK, 170, 40);
    draw_text("Wave: " + to_string(game.wave), COLOR_BLACK, 320, 40);

    // Draw enemy path
    for (int i = 0; i < length(game.path) - 1; i++)
    {
        draw_line(COLOR_GRAY, game.path[i], game.path[i + 1]);
    }

    // Draw available tower spots and placed towers
    draw_tower_spots(game.tower_spots);
    draw_towers(game.towers, game.tower_spots);

    // Draw enemies and projectiles
    draw_enemies(game.enemies);
    draw_projectiles(game.projectiles);

    // Draw tower menus
    if (game.purchase_menu_open)
    {
        draw_purchase_menu(game);
    }

    if (game.management_menu_open)
    {
        draw_management_menu(game);
    }

    // Display game over message
    if (game.lives <= 0)
    {
        draw_text("GAME OVER", COLOR_RED, 450, 350);
    }
}