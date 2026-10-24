#include "config.h"
#include "tower.h"
#include "projectile.h"

static tower create_tower(
    const tower_stats &stats,
    int id,
    int spot_id
)
{
    tower new_tower{};

    new_tower.id = id;
    new_tower.spot_id = spot_id;
    new_tower.type = stats.type;

    new_tower.targeting = FIRST;
    new_tower.level = 1;
    new_tower.last_attack_time = 0.0;

    new_tower.cost = stats.cost;
    new_tower.damage = stats.damage;
    new_tower.range = stats.range;
    new_tower.attack_delay = stats.attack_delay;

    return new_tower;
}

bool place_tower(game_data &game, int spot_index, tower_type type)
{
    // Validate the tower spot index
    if (spot_index < 0 ||
        spot_index >= length(game.tower_spots))
    {
        return false;
    }

    // Check if the spot is occupied
    if (game.tower_spots[spot_index].occupied) return false;

    // Find the tower's configuration
    const tower_stats *stats = find_tower_stats(game.config, type);
    if (stats == nullptr) return false;

    tower new_tower = create_tower(
        *stats,
        game.next_tower_id,
        game.tower_spots[spot_index].id
    );

    // Check if the player has enough coins
    if (game.coins < new_tower.cost) return false;

    game.next_tower_id++;

    add(game.towers, new_tower);

    // Mark the tower spot as occupied
    game.tower_spots[spot_index].occupied = true;
    game.tower_spots[spot_index].tower_id = new_tower.id;

    // Deduct the tower cost
    game.coins -= new_tower.cost;

    return true;
}

void handle_tower_click(game_data &game)
{
    if (game.purchase_menu_open) return;
    if (game.management_menu_open) return;
    if (!mouse_clicked(LEFT_BUTTON)) return;

    point_2d mouse = mouse_position();

    for (int i = 0; i < length(game.tower_spots); i++)
    {
        circle spot_circle = circle_at(game.tower_spots[i].position, 20);

        if (point_in_circle(mouse, spot_circle))
        {
            if (!game.tower_spots[i].occupied)
            {
                game.purchase_menu_open = true;
            }
            else
            {
                game.management_menu_open = true;
            }

            game.selected_spot_index = i;
            break;
        }
    }
}

static bool enemy_in_range(
    const tower &current_tower,
    const tower_spot &spot,
    const enemy &current_enemy
)
{
    circle range_circle = circle_at(spot.position, current_tower.range);

    return current_enemy.active && point_in_circle(current_enemy.position, range_circle);
}

// Return true if the checking enemy has higher targeting priority
static bool is_better_target(
    const tower &current_tower,
    const tower_spot &spot,
    const enemy &checking_enemy,
    const enemy &selected_enemy,
    const dynamic_array<point_2d> &path
)
{
    switch (current_tower.targeting)
    {
        case FIRST:
        {
            int checking_path_index = checking_enemy.path_index;
            int selected_path_index = selected_enemy.path_index;

            if (checking_path_index > selected_path_index)
            {
                return true;
            }
            else if (checking_path_index == selected_path_index)
            {
                // Compare distances to the next waypoint
                int next_index = checking_path_index + 1;

                if (next_index >= length(path)) return false;

                double checking_distance = point_point_distance(
                    checking_enemy.position,
                    path[next_index]
                );

                double selected_distance = point_point_distance(
                    selected_enemy.position,
                    path[next_index]
                );

                if (checking_distance < selected_distance) return true;
            }

            break;
        }

        case CLOSEST:
        {
            double checking_distance = point_point_distance(
                checking_enemy.position,
                spot.position
            );

            double selected_distance = point_point_distance(
                selected_enemy.position,
                spot.position
            );

            if (checking_distance < selected_distance) return true;

            break;
        }

        case STRONGEST:
        {
            if (checking_enemy.health > selected_enemy.health) return true;

            break;
        }

        case WEAKEST:
        {
            if (checking_enemy.health < selected_enemy.health) return true;

            break;
        }
    }

    return false;
}

static int find_target(
    const tower &current_tower,
    const tower_spot &spot,
    const dynamic_array<enemy> &enemies,
    const dynamic_array<point_2d> &path
)
{
    int selected_index = -1;

    // Find the best enemy within the tower's range
    for (int i = 0; i < length(enemies); i++)
    {
        if (!enemy_in_range(current_tower, spot, enemies[i])) continue;

        if (selected_index == -1)
        {
            selected_index = i;
            continue;
        }

        if (is_better_target(current_tower, spot, enemies[i], enemies[selected_index], path))
        {
            selected_index = i;
        }
    }

    if (selected_index == -1) return -1;

    return enemies[selected_index].id;
}

void update_tower_attacks(game_data &game, double current_time)
{
    for (int i = 0; i < length(game.towers); i++)
    {
        tower &current_tower = game.towers[i];

        for (int j = 0; j < length(game.tower_spots); j++)
        {
            const tower_spot &spot = game.tower_spots[j];

            if (current_tower.spot_id != spot.id) continue;

            // Find an enemy within the tower's range
            int target_enemy_id = find_target(
                current_tower,
                spot,
                game.enemies,
                game.path
            );

            if (target_enemy_id == -1) break;

            // Check whether the attack cooldown has finished
            if (current_time - current_tower.last_attack_time < current_tower.attack_delay) break;

            // Fire a projectile and reset the attack timer
            fire_projectile(
                game.projectiles,
                current_tower,
                spot,
                target_enemy_id
            );

            current_tower.last_attack_time = current_time;

            break;
        }
    }
}

void change_tower_targeting(game_data &game, targeting_mode mode)
{
    // Validate the selected tower spot index
    if (game.selected_spot_index < 0 ||
        game.selected_spot_index >= length(game.tower_spots)) return;

    int selected_tower_id = game.tower_spots[game.selected_spot_index].tower_id;

    for (int i = 0; i < length(game.towers); i++)
    {
        if (game.towers[i].id == selected_tower_id)
        {
            game.towers[i].targeting = mode;
            break;
        }
    }
}

int get_upgrade_cost(const game_data &game, const tower &current_tower)
{
    if (current_tower.level >= 5) return 0;

    const tower_upgrade *upgrade = find_tower_upgrade(
        game.config,
        current_tower.type,
        current_tower.level + 1
    );
    if (upgrade == nullptr) return 0;

    return upgrade->cost;
}

bool upgrade_tower(game_data &game)
{
    // Validate the selected tower spot
    if (game.selected_spot_index < 0 ||
        game.selected_spot_index >= length(game.tower_spots)) return false;

    if (!game.tower_spots[game.selected_spot_index].occupied) return false;

    int selected_tower_id = game.tower_spots[game.selected_spot_index].tower_id;

    // Find the selected tower
    for (int i = 0; i < length(game.towers); i++)
    {
        if (game.towers[i].id == selected_tower_id)
        {
            tower &current_tower = game.towers[i];

            // Check if the tower is already at maximum level
            if (current_tower.level >= 5) return false;

            const tower_upgrade *upgrade = find_tower_upgrade(
                game.config,
                current_tower.type,
                current_tower.level + 1
            );
            if (upgrade == nullptr) return false;

            if (game.coins < upgrade->cost) return false;

            game.coins -= upgrade->cost;

            current_tower.damage += upgrade->damage_increase;
            current_tower.range += upgrade->range_increase;
            current_tower.attack_delay -= upgrade->attack_delay_reduction;

            current_tower.level++;

            return true;
        }
    }

    return false;
}