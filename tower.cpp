#include "tower.h"
#include "config.h"
#include "projectile.h"

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
    new_tower.level = 1;
    new_tower.cost = stats.cost;
    new_tower.total_spent = stats.cost;
    new_tower.damage = stats.damage;
    new_tower.range = stats.range;
    new_tower.attack_delay = stats.attack_delay;
    new_tower.last_attack_time = 0.0;

    return new_tower;
}

bool place_tower(
    game_data &game,
    int spot_index,
    tower_type type)
{
    if (spot_index < 0 || spot_index >= length(game.tower_spots))
    {
        return false;
    }

    tower_spot &spot = game.tower_spots[spot_index];

    if (spot.occupied)
    {
        return false;
    }

    const tower_stats *stats = find_tower_stats(game.config, type);

    if (stats == nullptr)
    {
        return false;
    }

    if (game.coins < stats->cost)
    {
        return false;
    }

    tower new_tower = create_tower(
        *stats,
        game.next_tower_id,
        spot.id);

    game.next_tower_id++;

    add(game.towers, new_tower);

    spot.occupied = true;
    spot.tower_id = new_tower.id;

    game.coins -= stats->cost;

    return true;
}

static bool enemy_in_range(
    const enemy &current_enemy,
    point_2d tower_position,
    double range)
{
    return point_point_distance(
               current_enemy.position,
               tower_position) <= range;
}

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
        if (checking_enemy.path_index != selected_enemy.path_index)
        {
            return checking_enemy.path_index > selected_enemy.path_index;
        }

        int next_index = checking_enemy.path_index + 1;

        if (next_index >= length(path))
        {
            return false;
        }

        return point_point_distance(
                   checking_enemy.position,
                   path[next_index]) < point_point_distance(selected_enemy.position,
                                                            path[next_index]);
    }

    case CLOSEST:
        return point_point_distance(
                   checking_enemy.position,
                   tower_position) < point_point_distance(selected_enemy.position,
                                                          tower_position);

    case STRONGEST:
        return checking_enemy.health > selected_enemy.health;

    case WEAKEST:
        return checking_enemy.health < selected_enemy.health;
    }

    return false;
}

static int find_target(
    const game_data &game,
    const tower &current_tower,
    const tower_spot &spot)
{
    int best_index = -1;

    for (int i = 0; i < length(game.enemies); i++)
    {
        const enemy &checking_enemy = game.enemies[i];

        if (!checking_enemy.active)
        {
            continue;
        }

        if (!enemy_in_range(
                checking_enemy,
                spot.position,
                current_tower.range))
        {
            continue;
        }

        if (best_index == -1 ||
            is_better_target(
                checking_enemy,
                game.enemies[best_index],
                current_tower.targeting,
                spot.position,
                game.path))
        {
            best_index = i;
        }
    }

    if (best_index == -1)
    {
        return -1;
    }

    return game.enemies[best_index].id;
}

int get_upgrade_cost(
    const game_data &game,
    const tower &current_tower)
{
    if (current_tower.level >= 5)
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

bool upgrade_tower(game_data &game)
{
    int spot_index = game.selected_spot_index;

    if (spot_index < 0 || spot_index >= length(game.tower_spots))
    {
        return false;
    }

    const tower_spot &spot = game.tower_spots[spot_index];

    if (!spot.occupied)
    {
        return false;
    }

    for (int i = 0; i < length(game.towers); i++)
    {
        tower &current_tower = game.towers[i];

        if (current_tower.id != spot.tower_id)
        {
            continue;
        }

        if (current_tower.level >= 5)
        {
            return false;
        }

        const tower_upgrade *upgrade = find_tower_upgrade(
            game.config,
            current_tower.type,
            current_tower.level + 1);

        if (upgrade == nullptr)
        {
            return false;
        }

        if (game.coins < upgrade->cost)
        {
            return false;
        }

        game.coins -= upgrade->cost;

        current_tower.total_spent += upgrade->cost;
        current_tower.damage += upgrade->damage_increase;
        current_tower.range += upgrade->range_increase;
        current_tower.attack_delay -= upgrade->attack_delay_reduction;
        current_tower.level++;

        return true;
    }

    return false;
}

int get_sell_refund(
    const tower &current_tower)
{
    return current_tower.total_spent * 70 / 100;
}

bool sell_tower(game_data &game)
{
    int spot_index = game.selected_spot_index;

    if (spot_index < 0 || spot_index >= length(game.tower_spots))
    {
        return false;
    }

    tower_spot &spot = game.tower_spots[spot_index];

    if (!spot.occupied)
    {
        return false;
    }

    for (int i = 0; i < length(game.towers); i++)
    {
        if (game.towers[i].id != spot.tower_id)
        {
            continue;
        }

        game.coins += get_sell_refund(game.towers[i]);

        remove(game.towers, i);

        spot.occupied = false;
        spot.tower_id = -1;

        return true;
    }

    return false;
}

void change_tower_targeting(
    game_data &game,
    targeting_mode mode)
{
    int spot_index = game.selected_spot_index;

    if (spot_index < 0 || spot_index >= length(game.tower_spots))
    {
        return;
    }

    const tower_spot &spot = game.tower_spots[spot_index];

    if (!spot.occupied)
    {
        return;
    }

    for (int i = 0; i < length(game.towers); i++)
    {
        if (game.towers[i].id == spot.tower_id)
        {
            game.towers[i].targeting = mode;
            return;
        }
    }
}

void handle_tower_click(game_data &game)
{
    if (game.purchase_menu_open || game.management_menu_open)
    {
        return;
    }

    if (!mouse_clicked(LEFT_BUTTON))
    {
        return;
    }

    point_2d mouse = mouse_position();

    for (int i = 0; i < length(game.tower_spots); i++)
    {
        const tower_spot &spot = game.tower_spots[i];

        if (point_point_distance(mouse, spot.position) <= 20)
        {
            game.selected_spot_index = i;

            if (spot.occupied)
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

void update_tower_attacks(
    game_data &game,
    double current_time)
{
    for (int i = 0; i < length(game.towers); i++)
    {
        tower &current_tower = game.towers[i];

        if (current_time - current_tower.last_attack_time <
            current_tower.attack_delay)
        {
            continue;
        }

        for (int j = 0; j < length(game.tower_spots); j++)
        {
            const tower_spot &spot = game.tower_spots[j];

            if (current_tower.spot_id != spot.id)
            {
                continue;
            }

            int target_enemy_id = find_target(
                game,
                current_tower,
                spot);

            if (target_enemy_id != -1)
            {
                fire_projectile(
                    game.projectiles,
                    current_tower,
                    spot,
                    target_enemy_id);

                current_tower.last_attack_time = current_time;
            }

            break;
        }
    }
}