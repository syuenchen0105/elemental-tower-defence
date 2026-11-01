#include "config.h"
#include "enemy.h"
#include "render.h"
#include "tower.h"
#include "wave.h"

// Shared drawing and layout constants
const double WINDOW_WIDTH = 1000;
const double WINDOW_HEIGHT = 700;
const double TOWER_SPOT_RADIUS = 20;
const double TOWER_RADIUS = 15;
const double PROJECTILE_RADIUS = 5;

const double NORMAL_ENEMY_RADIUS = 12;
const double FAST_ENEMY_RADIUS = 9;
const double TANK_ENEMY_RADIUS = 16;
const double SWARM_ENEMY_RADIUS = 8;
const double BOSS_ENEMY_RADIUS = 23;
const double HEALTH_BAR_WIDTH = 34;
const double BOSS_HEALTH_BAR_WIDTH = 50;
const double HEALTH_BAR_HEIGHT = 5;
const double HEALTH_BAR_GAP = 12;

const double MENU_WIDTH = 180;
const double MENU_GAP = 30;
const double MENU_SIDE_PADDING = 15;
const double MENU_BUTTON_HEIGHT = 30;
const double MENU_FIRST_BUTTON_OFFSET = 30;
const double MENU_BUTTON_SPACING = 40;
const double PURCHASE_MENU_HEIGHT = 240;
const double MANAGEMENT_MENU_HEIGHT = 320;

const double PREPARATION_PANEL_X = 550;
const double PREPARATION_PANEL_Y = 10;
const double PREPARATION_PANEL_WIDTH = 440;
const double PREPARATION_PANEL_HEIGHT = 85;
const double START_BUTTON_WIDTH = 145;
const double START_BUTTON_HEIGHT = 32;
const double START_BUTTON_X_OFFSET = 280;

// Return the display colour for each enemy type
static color get_enemy_color(enemy_type type)
{
    switch (type)
    {
    case NORMAL:
        return COLOR_RED;

    case FAST:
        return COLOR_ORANGE;

    case TANK:
        return COLOR_BROWN;

    case SWARM:
        return rgb_color(160, 75, 200);

    case BOSS:
        return rgb_color(130, 25, 35);
    }

    return COLOR_RED;
}

// Return the drawing radius for each enemy type
static double get_enemy_radius(enemy_type type)
{
    switch (type)
    {
    case NORMAL:
        return NORMAL_ENEMY_RADIUS;

    case FAST:
        return FAST_ENEMY_RADIUS;

    case TANK:
        return TANK_ENEMY_RADIUS;

    case SWARM:
        return SWARM_ENEMY_RADIUS;

    case BOSS:
        return BOSS_ENEMY_RADIUS;
    }

    return NORMAL_ENEMY_RADIUS;
}

// Draw all available tower placement spots
void draw_tower_spots(const dynamic_array<tower_spot> &spots)
{
    for (int i = 0; i < length(spots); i++)
    {
        if (!spots[i].occupied)
        {
            fill_circle(
                COLOR_BLUE,
                spots[i].position,
                TOWER_SPOT_RADIUS);
        }
    }
}

// Draw each purchased tower at its corresponding placement spot
void draw_towers(
    const dynamic_array<tower> &towers,
    const dynamic_array<tower_spot> &spots)
{
    for (int i = 0; i < length(towers); i++)
    {
        for (int j = 0; j < length(spots); j++)
        {
            if (towers[i].spot_id == spots[j].id)
            {
                color tower_color = COLOR_WHITE;

                // Choose a colour based on the tower's element
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

                fill_circle(
                    tower_color,
                    spots[j].position,
                    TOWER_RADIUS);

                break;
            }
        }
    }
}

// Draw active enemies with different appearances and health bars
void draw_enemies(const dynamic_array<enemy> &enemies)
{
    for (int i = 0; i < length(enemies); i++)
    {
        const enemy &current_enemy = enemies[i];

        if (!current_enemy.active)
        {
            continue;
        }

        color enemy_color = get_enemy_color(current_enemy.type);
        double radius = get_enemy_radius(current_enemy.type);

        // Draw the enemy using its type-specific colour and size
        fill_circle(
            enemy_color,
            current_enemy.position,
            radius);

        // Calculate the proportion of health remaining
        double health_ratio = 0.0;

        if (current_enemy.max_health > 0)
        {
            health_ratio =
                current_enemy.health * 1.0 / current_enemy.max_health;
        }

        // Keep the health ratio between zero and one
        if (health_ratio < 0.0)
        {
            health_ratio = 0.0;
        }

        if (health_ratio > 1.0)
        {
            health_ratio = 1.0;
        }

        // Use a wider health bar for boss enemies
        double bar_width = HEALTH_BAR_WIDTH;

        if (current_enemy.type == BOSS)
        {
            bar_width = BOSS_HEALTH_BAR_WIDTH;
        }

        double bar_x = current_enemy.position.x - bar_width / 2;
        double bar_y = current_enemy.position.y - radius - HEALTH_BAR_GAP;

        // Draw the health bar background
        fill_rectangle(
            COLOR_BLACK,
            bar_x,
            bar_y,
            bar_width,
            HEALTH_BAR_HEIGHT);

        // Draw the remaining health
        fill_rectangle(
            COLOR_GREEN,
            bar_x,
            bar_y,
            bar_width * health_ratio,
            HEALTH_BAR_HEIGHT);
    }
}

// Display one active status letter inside each enemy circle
static void draw_enemy_status_indicators(
    const dynamic_array<enemy> &enemies,
    double current_time)
{
    for (int i = 0; i < length(enemies); i++)
    {
        const enemy &current_enemy = enemies[i];

        if (!current_enemy.active)
        {
            continue;
        }

        string status_text = "";

        // Prioritise movement restrictions over elemental effects
        if (current_time < current_enemy.effects.stun_end_time)
        {
            status_text = "S";
        }
        else if (current_time < current_enemy.effects.immobilize_end_time)
        {
            status_text = "I";
        }
        else if (current_time < current_enemy.effects.end_times[BURNING])
        {
            status_text = "B";
        }
        else if (current_time < current_enemy.effects.end_times[WET])
        {
            status_text = "W";
        }
        else if (current_time < current_enemy.effects.end_times[CHARGED])
        {
            status_text = "C";
        }
        else if (current_time < current_enemy.effects.end_times[CRACKED])
        {
            status_text = "K";
        }

        // Draw the status letter at the centre of the enemy
        if (status_text != "")
        {
            draw_text(
                status_text,
                COLOR_WHITE,
                current_enemy.position.x - 4,
                current_enemy.position.y - 8);
        }
    }
}

// Draw all active projectiles
void draw_projectiles(
    const dynamic_array<projectile> &projectiles)
{
    for (int i = 0; i < length(projectiles); i++)
    {
        if (!projectiles[i].active)
        {
            continue;
        }

        fill_circle(
            COLOR_YELLOW,
            projectiles[i].position,
            PROJECTILE_RADIUS);
    }
}

// Purchase a selected tower and close the menu on success
static void select_tower(game_data &game, tower_type type)
{
    if (place_tower(game, game.selected_spot_index, type))
    {
        game.purchase_menu_open = false;
        game.selected_spot_index = -1;
    }
}

// Change the selected tower's targeting strategy
static void select_targeting_mode(
    game_data &game,
    targeting_mode mode)
{
    change_tower_targeting(game, mode);

    game.management_menu_open = false;
    game.selected_spot_index = -1;
}

// Upgrade the selected tower and close the menu on success
static void select_upgrade(game_data &game)
{
    if (upgrade_tower(game))
    {
        game.management_menu_open = false;
        game.selected_spot_index = -1;
    }
}

// Sell the selected tower and close the menu on success
static void select_sell(game_data &game)
{
    if (sell_tower(game))
    {
        game.management_menu_open = false;
        game.selected_spot_index = -1;
    }
}

// Draw a purchase option using the tower cost from configuration
static void draw_purchase_option(
    game_data &game,
    tower_type type,
    const string &name,
    rectangle button_area)
{
    const tower_stats *stats = find_tower_stats(
        game.config,
        type);

    if (stats == nullptr)
    {
        return;
    }

    if (button(
            name + " - " + to_string(stats->cost),
            button_area))
    {
        select_tower(game, type);
    }
}

// Calculate a menu position that stays inside the game window
static point_2d get_menu_position(
    point_2d spot_position,
    double menu_height)
{
    point_2d position{};

    // Place the menu beside the tower spot
    position.x = spot_position.x + MENU_GAP;

    // Move the menu to the left if it exceeds the window boundary
    if (position.x + MENU_WIDTH > WINDOW_WIDTH)
    {
        position.x = spot_position.x - MENU_GAP - MENU_WIDTH;
    }

    // Centre the menu vertically beside the tower spot
    position.y = spot_position.y - menu_height / 2;

    // Keep the menu inside the window vertically
    if (position.y < 0)
    {
        position.y = 0;
    }

    if (position.y + menu_height > WINDOW_HEIGHT)
    {
        position.y = WINDOW_HEIGHT - menu_height;
    }

    return position;
}

// Draw the white background and black border shared by both tower menus
static void draw_menu_background(
    point_2d position,
    double menu_height)
{
    fill_rectangle(
        COLOR_WHITE,
        position.x,
        position.y,
        MENU_WIDTH,
        menu_height);

    draw_rectangle(
        COLOR_BLACK,
        position.x,
        position.y,
        MENU_WIDTH,
        menu_height);
}

// Create a consistently spaced button area inside a menu
static rectangle get_menu_button_area(
    point_2d menu_position,
    int button_index)
{
    return rectangle_from(
        menu_position.x + MENU_SIDE_PADDING,
        menu_position.y + MENU_FIRST_BUTTON_OFFSET +
            button_index * MENU_BUTTON_SPACING,
        MENU_WIDTH - MENU_SIDE_PADDING * 2,
        MENU_BUTTON_HEIGHT);
}

// Draw a targeting button and apply the selected targeting mode
static void draw_targeting_button(
    game_data &game,
    targeting_mode mode,
    const string &label,
    rectangle button_area)
{
    if (button(label, button_area))
    {
        select_targeting_mode(game, mode);
    }
}

// Display the tower purchase menu next to the selected spot
void draw_purchase_menu(game_data &game)
{
    if (
        game.selected_spot_index < 0 ||
        game.selected_spot_index >= length(game.tower_spots))
    {
        return;
    }

    point_2d spot_position =
        game.tower_spots[game.selected_spot_index].position;

    // Reuse the same menu positioning and background drawing functions
    point_2d menu_position = get_menu_position(
        spot_position,
        PURCHASE_MENU_HEIGHT);

    draw_menu_background(
        menu_position,
        PURCHASE_MENU_HEIGHT);

    draw_text(
        "Choose Tower",
        COLOR_BLACK,
        menu_position.x + 15,
        menu_position.y + 15);

    // Draw the four elemental tower purchase options
    draw_purchase_option(
        game,
        FIRE,
        "FIRE",
        get_menu_button_area(menu_position, 0));

    draw_purchase_option(
        game,
        WATER,
        "WATER",
        get_menu_button_area(menu_position, 1));

    draw_purchase_option(
        game,
        ELECTRIC,
        "ELECTRIC",
        get_menu_button_area(menu_position, 2));

    draw_purchase_option(
        game,
        EARTH,
        "EARTH",
        get_menu_button_area(menu_position, 3));

    // Allow the player to close the purchase menu
    if (button(
            "CANCEL",
            get_menu_button_area(menu_position, 4)))
    {
        game.purchase_menu_open = false;
        game.selected_spot_index = -1;
    }
}

// Display targeting, upgrading and selling options for a selected tower
void draw_management_menu(game_data &game)
{
    if (
        game.selected_spot_index < 0 ||
        game.selected_spot_index >= length(game.tower_spots))
    {
        return;
    }

    int selected_tower_id =
        game.tower_spots[game.selected_spot_index].tower_id;

    int upgrade_cost = 0;
    int sell_refund = 0;
    int tower_level = 0;

    // Find the selected tower and retrieve its information
    for (int i = 0; i < length(game.towers); i++)
    {
        if (game.towers[i].id == selected_tower_id)
        {
            upgrade_cost = get_upgrade_cost(
                game,
                game.towers[i]);

            sell_refund = get_sell_refund(
                game.towers[i]);

            tower_level = game.towers[i].level;
            break;
        }
    }

    if (tower_level == 0)
    {
        return;
    }

    point_2d spot_position =
        game.tower_spots[game.selected_spot_index].position;

    // Reuse the same menu positioning and background drawing functions
    point_2d menu_position = get_menu_position(
        spot_position,
        MANAGEMENT_MENU_HEIGHT);

    draw_menu_background(
        menu_position,
        MANAGEMENT_MENU_HEIGHT);

    draw_text(
        "Tower - Lv " + to_string(tower_level),
        COLOR_BLACK,
        menu_position.x + 15,
        menu_position.y + 15);

    // Draw the four targeting options using one helper function
    draw_targeting_button(
        game,
        FIRST,
        "FIRST",
        get_menu_button_area(menu_position, 0));

    draw_targeting_button(
        game,
        CLOSEST,
        "CLOSEST",
        get_menu_button_area(menu_position, 1));

    draw_targeting_button(
        game,
        STRONGEST,
        "STRONGEST",
        get_menu_button_area(menu_position, 2));

    draw_targeting_button(
        game,
        WEAKEST,
        "WEAKEST",
        get_menu_button_area(menu_position, 3));

    // Display the next upgrade cost or maximum level
    string upgrade_text = "MAX LEVEL";

    if (upgrade_cost > 0)
    {
        upgrade_text = "UPGRADE - " + to_string(upgrade_cost);
    }

    if (button(
            upgrade_text,
            get_menu_button_area(menu_position, 4)))
    {
        if (upgrade_cost > 0)
        {
            select_upgrade(game);
        }
    }

    // Display the selling refund for the selected tower
    string sell_text = "SELL - " + to_string(sell_refund);

    if (button(
            sell_text,
            get_menu_button_area(menu_position, 5)))
    {
        select_sell(game);
    }

    // Allow the player to close the management menu
    if (button(
            "CANCEL",
            get_menu_button_area(menu_position, 6)))
    {
        game.management_menu_open = false;
        game.selected_spot_index = -1;
    }
}

// Build a readable preview for one enemy type
static string get_preview_text(
    const fixed_array<int, 5> &counts,
    enemy_type type,
    const string &name)
{
    if (counts[type] == 0)
    {
        return "";
    }

    return name + " x" + to_string(counts[type]) + "   ";
}

// Show upcoming enemies and allow the player to start the wave early
static void draw_preparation_panel(game_data &game)
{
    // Get the enemy count from wave.json configuration
    int enemies_per_wave = get_enemies_per_wave(game);

    fixed_array<int, 5> counts{0};

    // Count each enemy type in the upcoming wave
    for (int i = 0; i < enemies_per_wave; i++)
    {
        enemy_type type = get_wave_enemy_type(
            game.wave,
            i,
            enemies_per_wave);

        counts[type]++;
    }

    double remaining_time =
        game.preparation_end_time - game.current_time;

    if (remaining_time < 0.0)
    {
        remaining_time = 0.0;
    }

    // Round up the remaining preparation time
    int countdown = remaining_time;

    if (remaining_time > countdown)
    {
        countdown++;
    }

    // Draw the preparation panel background and border
    fill_rectangle(
        rgb_color(235, 245, 255),
        PREPARATION_PANEL_X,
        PREPARATION_PANEL_Y,
        PREPARATION_PANEL_WIDTH,
        PREPARATION_PANEL_HEIGHT);

    draw_rectangle(
        COLOR_BLUE,
        PREPARATION_PANEL_X,
        PREPARATION_PANEL_Y,
        PREPARATION_PANEL_WIDTH,
        PREPARATION_PANEL_HEIGHT);

    // Display the upcoming wave and countdown
    draw_text(
        "PREPARE WAVE " + to_string(game.wave) +
            " - " + to_string(countdown) + "s",
        COLOR_BLACK,
        PREPARATION_PANEL_X + 10,
        PREPARATION_PANEL_Y + 7);

    // Build the preview using the same rules as enemy spawning
    string first_row =
        get_preview_text(counts, NORMAL, "NORMAL") +
        get_preview_text(counts, FAST, "FAST") +
        get_preview_text(counts, TANK, "TANK");

    string second_row =
        get_preview_text(counts, SWARM, "SWARM") +
        get_preview_text(counts, BOSS, "BOSS");

    // Draw the upcoming enemy types
    draw_text(
        first_row,
        COLOR_BLACK,
        PREPARATION_PANEL_X + 10,
        PREPARATION_PANEL_Y + 29);

    draw_text(
        second_row,
        COLOR_BLACK,
        PREPARATION_PANEL_X + 10,
        PREPARATION_PANEL_Y + 53);

    // Centre the START EARLY button vertically on the right
    double button_x = PREPARATION_PANEL_X + START_BUTTON_X_OFFSET;
    double button_y = PREPARATION_PANEL_Y +
                      (PREPARATION_PANEL_HEIGHT - START_BUTTON_HEIGHT) / 2;

    if (button(
            "START EARLY",
            rectangle_from(
                button_x,
                button_y,
                START_BUTTON_WIDTH,
                START_BUTTON_HEIGHT)))
    {
        game.phase = IN_WAVE;
    }
}

// Draw the complete game world, interface and menus
void draw_game(game_data &game)
{
    clear_screen(COLOR_WHITE);

    // Draw the game information
    draw_text(
        "Tower Defence",
        COLOR_BLACK,
        20,
        20);

    draw_text(
        "Coins: " + to_string(game.coins),
        COLOR_BLACK,
        20,
        40);

    draw_text(
        "Lives: " + to_string(game.lives),
        COLOR_BLACK,
        170,
        40);

    draw_text(
        "Wave: " + to_string(game.wave),
        COLOR_BLACK,
        320,
        40);

    // Draw the enemy movement path
    for (int i = 0; i < length(game.path) - 1; i++)
    {
        draw_line(
            COLOR_GRAY,
            game.path[i],
            game.path[i + 1]);
    }

    // Draw the available tower spots and purchased towers
    draw_tower_spots(game.tower_spots);
    draw_towers(game.towers, game.tower_spots);

    // Draw enemies and their health bars
    draw_enemies(game.enemies);

    // Draw one status letter inside each affected enemy
    draw_enemy_status_indicators(
        game.enemies,
        game.current_time);

    // Draw all active projectiles
    draw_projectiles(game.projectiles);

    // Display wave preparation information when menus are closed
    if (
        game.lives > 0 &&
        game.phase == PREPARATION &&
        !game.purchase_menu_open &&
        !game.management_menu_open)
    {
        draw_preparation_panel(game);
    }

    // Draw the tower purchase menu
    if (game.purchase_menu_open)
    {
        draw_purchase_menu(game);
    }

    // Draw the tower management menu
    if (game.management_menu_open)
    {
        draw_management_menu(game);
    }

    // Display the game-over message
    if (game.lives <= 0)
    {
        draw_text(
            "GAME OVER",
            COLOR_RED,
            450,
            350);
    }
}