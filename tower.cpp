#include "tower.h"
#include "config.h"
#include "projectile.h"

// Shared constants for tower operations
const int STARTING_TOWER_LEVEL = 1;
const int MAX_TOWER_LEVEL = 5;
const int SELL_REFUND_PERCENT = 70;
const int PERCENT_DIVISOR = 100;
const double TOWER_SPOT_CLICK_RADIUS = 20.0;

// Create a level 1 tower using its configuration statistics
static tower create_tower(
    const tower_stats &stats,
    int id,
    int spot_id)
{
    tower new_tower{};

    new_tower.id = id;
    new_tower.spot_id = spot_id;
    new_tower.type = stats.type;
    new_tower.targeting = FIRST;

    new_tower.level = STARTING_TOWER_LEVEL;
    new_tower.cost = stats.cost;
    new_tower.total_spent = stats.cost;

    new_tower.damage = stats.damage;
    new_tower.range = stats.range;
    new_tower.attack_delay = stats.attack_delay;
    new_tower.last_attack_time = 0.0;

    return new_tower;
}

// Place a tower at an available spot if the player has enough coins
bool place_tower(
    game_data &game,
    int spot_index,
    tower_type type)
{
    // Check whether the selected spot index is valid
    if (
        spot_index < 0 ||
        spot_index >= length(game.tower_spots))
    {
        return false;
    }

    tower_spot &spot = game.tower_spots[spot_index];

    // Prevent placing multiple towers at the same spot
    if (spot.occupied)
    {
        return false;
    }

    // Find the selected tower type in the JSON configuration
    const tower_stats *stats = find_tower_stats(
        game.config,
        type);

    if (stats == nullptr)
    {
        return false;
    }

    // Reject the purchase if the player has insufficient coins
    if (game.coins < stats->cost)
    {
        return false;
    }

    // Create the tower with a unique ID
    tower new_tower = create_tower(
        *stats,
        game.next_tower_id,
        spot.id);

    game.next_tower_id++;

    // Add the new tower to the dynamic array
    add(game.towers, new_tower);

    // Mark the selected spot as occupied
    spot.occupied = true;
    spot.tower_id = new_tower.id;

    // Deduct the tower's purchase cost
    game.coins -= stats->cost;

    return true;
}

// Check whether an active enemy is within a tower's attack range
static bool enemy_in_range(
    const enemy &current_enemy,
    point_2d tower_position,
    double range)
{
    if (!current_enemy.active)
    {
        return false;
    }

    double distance = point_point_distance(
        current_enemy.position,
        tower_position);

    return distance <= range;
}

// Compare two enemies according to the tower's targeting strategy
static bool is_better_target(
    const enemy &checking_enemy,
    const enemy &selected_enemy,
    targeting_mode mode,
    point_2d tower_position,
    const dynamic_array<point_2d> &path)
{
    switch (mode)
    {
    case FIRST:
    {
        // Prioritise the enemy furthest along the path
        if (checking_enemy.path_index > selected_enemy.path_index)
        {
            return true;
        }

        if (checking_enemy.path_index < selected_enemy.path_index)
        {
            return false;
        }

        // On the same segment, prefer the enemy closer to the next waypoint
        int next_path_index = checking_enemy.path_index + 1;

        if (next_path_index < length(path))
        {
            double checking_distance = point_point_distance(
                checking_enemy.position,
                path[next_path_index]);

            double selected_distance = point_point_distance(
                selected_enemy.position,
                path[next_path_index]);

            return checking_distance < selected_distance;
        }

        return false;
    }

    case CLOSEST:
    {
        // Prioritise the enemy physically closest to the tower
        double checking_distance = point_point_distance(
            checking_enemy.position,
            tower_position);

        double selected_distance = point_point_distance(
            selected_enemy.position,
            tower_position);

        return checking_distance < selected_distance;
    }

    case STRONGEST:
    {
        // Prioritise the enemy with the highest current health
        return checking_enemy.health > selected_enemy.health;
    }

    case WEAKEST:
    {
        // Prioritise the enemy with the lowest current health
        return checking_enemy.health < selected_enemy.health;
    }
    }

    return false;
}

// Search for the highest-priority active enemy within tower range
static int find_target(
    const game_data &game,
    const tower &current_tower,
    const tower_spot &spot)
{
    int selected_index = -1;

    for (int i = 0; i < length(game.enemies); i++)
    {
        const enemy &checking_enemy = game.enemies[i];

        // Ignore inactive enemies and enemies outside attack range
        if (!enemy_in_range(
                checking_enemy,
                spot.position,
                current_tower.range))
        {
            continue;
        }

        // Select the first valid enemy as the initial target
        if (selected_index == -1)
        {
            selected_index = i;
            continue;
        }

        // Replace the target when another enemy has higher priority
        if (is_better_target(
                checking_enemy,
                game.enemies[selected_index],
                current_tower.targeting,
                spot.position,
                game.path))
        {
            selected_index = i;
        }
    }

    // Return -1 when no valid target is available
    if (selected_index == -1)
    {
        return -1;
    }

    return game.enemies[selected_index].id;
}

// Find a tower in the dynamic array using its unique ID
static int find_tower_index(
    const dynamic_array<tower> &towers,
    int tower_id)
{
    for (int i = 0; i < length(towers); i++)
    {
        if (towers[i].id == tower_id)
        {
            return i;
        }
    }

    return -1;
}

// Retrieve the cost of the selected tower's next upgrade
int get_upgrade_cost(
    const game_data &game,
    const tower &current_tower)
{
    // Prevent upgrades beyond the maximum level
    if (current_tower.level >= MAX_TOWER_LEVEL)
    {
        return 0;
    }

    const tower_upgrade *upgrade = find_tower_upgrade(
        game.config,
        current_tower.type,
        current_tower.level + 1);

    if (upgrade == nullptr)
    {
        return 0;
    }

    return upgrade->cost;
}

// Upgrade the selected tower if an upgrade exists and coins are sufficient
bool upgrade_tower(game_data &game)
{
    // Check whether a valid tower spot is selected
    if (
        game.selected_spot_index < 0 ||
        game.selected_spot_index >= length(game.tower_spots))
    {
        return false;
    }

    int selected_tower_id =
        game.tower_spots[game.selected_spot_index].tower_id;

    // Use the shared helper to find the selected tower
    int tower_index = find_tower_index(
        game.towers,
        selected_tower_id);

    if (tower_index == -1)
    {
        return false;
    }

    tower &current_tower = game.towers[tower_index];

    // Prevent upgrading beyond the maximum level
    if (current_tower.level >= MAX_TOWER_LEVEL)
    {
        return false;
    }

    // Find the next upgrade in the JSON configuration
    const tower_upgrade *upgrade = find_tower_upgrade(
        game.config,
        current_tower.type,
        current_tower.level + 1);

    if (upgrade == nullptr)
    {
        return false;
    }

    // Check whether the player has enough coins
    if (game.coins < upgrade->cost)
    {
        return false;
    }

    // Deduct coins and track the amount spent for selling
    game.coins -= upgrade->cost;
    current_tower.total_spent += upgrade->cost;

    // Apply the upgrade's damage, range and attack speed changes
    current_tower.damage += upgrade->damage_increase;
    current_tower.range += upgrade->range_increase;
    current_tower.attack_delay -= upgrade->attack_delay_reduction;

    // Increase the tower level
    current_tower.level++;

    return true;
}

// Calculate the selling refund from the total amount spent
int get_sell_refund(const tower &current_tower)
{
    // Integer division rounds the refund down
    return current_tower.total_spent *
           SELL_REFUND_PERCENT /
           PERCENT_DIVISOR;
}

// Sell the selected tower and make its placement spot available again
bool sell_tower(game_data &game)
{
    // Check whether the selected spot is valid
    if (
        game.selected_spot_index < 0 ||
        game.selected_spot_index >= length(game.tower_spots))
    {
        return false;
    }

    tower_spot &spot = game.tower_spots[game.selected_spot_index];

    // Only occupied spots can contain a tower to sell
    if (!spot.occupied)
    {
        return false;
    }

    // Find the selected tower with the shared helper
    int tower_index = find_tower_index(game.towers, spot.tower_id);

    if (tower_index == -1)
    {
        return false;
    }

    // Refund a percentage of the total coins invested
    int refund = get_sell_refund(game.towers[tower_index]);
    game.coins += refund;

    // Remove the tower and make its spot available again
    remove(game.towers, tower_index);
    spot.occupied = false;
    spot.tower_id = -1;

    return true;
}

// Change the targeting strategy of the currently selected tower
void change_tower_targeting(
    game_data &game,
    targeting_mode mode)
{
    // Ignore invalid tower spot selections
    if (
        game.selected_spot_index < 0 ||
        game.selected_spot_index >= length(game.tower_spots))
    {
        return;
    }

    int selected_tower_id =
        game.tower_spots[game.selected_spot_index].tower_id;

    // Use the shared helper to locate the tower
    int tower_index = find_tower_index(
        game.towers,
        selected_tower_id);

    if (tower_index != -1)
    {
        game.towers[tower_index].targeting = mode;
    }
}

// Handle mouse clicks on tower spots to open purchase or management menus
void handle_tower_click(game_data &game)
{
    // Prevent selecting another spot while a menu is open
    if (game.purchase_menu_open || game.management_menu_open)
    {
        return;
    }

    // Only process a new left mouse button click
    if (!mouse_clicked(LEFT_BUTTON))
    {
        return;
    }

    point_2d mouse = mouse_position();

    // Search for the tower spot clicked by the player
    for (int i = 0; i < length(game.tower_spots); i++)
    {
        double distance = point_point_distance(
            mouse,
            game.tower_spots[i].position);

        if (distance <= TOWER_SPOT_CLICK_RADIUS)
        {
            game.selected_spot_index = i;

            // Open the appropriate menu based on occupancy
            if (game.tower_spots[i].occupied)
            {
                game.management_menu_open = true;
            }
            else
            {
                game.purchase_menu_open = true;
            }

            return;
        }
    }
}

// Update tower cooldowns and fire projectiles at valid targets
void update_tower_attacks(
    game_data &game,
    double current_time)
{
    for (int i = 0; i < length(game.towers); i++)
    {
        tower &current_tower = game.towers[i];

        // Wait until the tower's attack cooldown has expired
        if (
            current_time - current_tower.last_attack_time <
            current_tower.attack_delay)
        {
            continue;
        }

        // Find the placement spot belonging to this tower
        for (int j = 0; j < length(game.tower_spots); j++)
        {
            const tower_spot &spot = game.tower_spots[j];

            if (spot.id != current_tower.spot_id)
            {
                continue;
            }

            // Find an enemy using the selected targeting strategy
            int target_enemy_id = find_target(
                game,
                current_tower,
                spot);

            if (target_enemy_id != -1)
            {
                // Fire a projectile toward the selected enemy
                fire_projectile(
                    game.projectiles,
                    current_tower,
                    spot,
                    target_enemy_id);

                // Reset the tower's attack cooldown
                current_tower.last_attack_time = current_time;
            }

            break;
        }
    }
}