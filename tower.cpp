#include "tower.h"
#include "projectile.h"

tower create_tower(tower_type type, int id, int spot_id)
{
    tower new_tower = {};

    new_tower.id = id;
    new_tower.spot_id = spot_id;
    new_tower.type = type;
    new_tower.targeting = FIRST;
    new_tower.level = 1;
    new_tower.last_attack_time = 0.0;

    // Set tower attributes based on its type
    switch (type)
    {
        case BASIC:
            new_tower.cost = 30;
            new_tower.damage = 10;
            new_tower.range = 120;
            new_tower.attack_delay = 1.0;
            break;

        case FIRE:
            new_tower.cost = 50;
            new_tower.damage = 18;
            new_tower.range = 110;
            new_tower.attack_delay = 1.2;
            break;

        case RAPID:
            new_tower.cost = 60;
            new_tower.damage = 8;
            new_tower.range = 100;
            new_tower.attack_delay = 0.4;
            break;

        case HEAVY:
            new_tower.cost = 80;
            new_tower.damage = 45;
            new_tower.range = 150;
            new_tower.attack_delay = 1.8;
            break;
    }

    return new_tower;
}

bool place_tower(game_data &game, int spot_index, tower_type type)
{
    // Validate the tower spot index
    if (spot_index < 0 || spot_index >= length(game.tower_spots))
    {
        return false;
    }

    // Check if the spot is occupied
    if (game.tower_spots[spot_index].occupied) return false;

    tower new_tower = create_tower(
        type,
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

bool enemy_in_range(
    const tower &current_tower,
    const tower_spot &spot,
    const enemy &current_enemy
)
{
    circle range_circle = circle_at(spot.position, current_tower.range);

    return current_enemy.active && point_in_circle(current_enemy.position, range_circle);
}

int find_target(
    const tower &current_tower,
    const tower_spot &spot,
    const dynamic_array<enemy> &enemies,
    const dynamic_array<point_2d> &path
)
{
    int selected_index = -1;

    switch (current_tower.targeting)
    {
        case FIRST:
        {
            // Find the enemy furthest along the path within range
            for (int i = 0; i < length(enemies); i++)
            {
                if (!enemy_in_range(current_tower, spot, enemies[i])) continue;

                if (selected_index == -1)
                {
                    selected_index = i;
                    continue;
                }

                int checking_path_index = enemies[i].path_index;
                int selected_path_index = enemies[selected_index].path_index;

                if (checking_path_index > selected_path_index)
                {
                    selected_index = i;
                }
                else if (checking_path_index == selected_path_index)
                {
                    // Compare distances to the next waypoint
                    int next_index = checking_path_index + 1;

                    if (next_index >= length(path)) continue;

                    double checking_distance = point_point_distance(
                        enemies[i].position,
                        path[next_index]
                    );

                    double selected_distance = point_point_distance(
                        enemies[selected_index].position,
                        path[next_index]
                    );

                    if (checking_distance < selected_distance)
                    {
                        selected_index = i;
                    }
                }
            }

            break;
        }

        case CLOSEST:
        {
            // Find the closest enemy within the tower's range
            for (int i = 0; i < length(enemies); i++)
            {
                if (!enemy_in_range(current_tower, spot, enemies[i])) continue;

                if (selected_index == -1)
                {
                    selected_index = i;
                    continue;
                }

                double checking_distance = point_point_distance(
                    enemies[i].position,
                    spot.position
                );

                double selected_distance = point_point_distance(
                    enemies[selected_index].position,
                    spot.position
                );

                if (checking_distance < selected_distance)
                {
                    selected_index = i;
                }
            }

            break;
        }

        case STRONGEST:
        {
            // Find the enemy with the highest health within range
            for (int i = 0; i < length(enemies); i++)
            {
                if (!enemy_in_range(current_tower, spot, enemies[i])) continue;

                if (selected_index == -1)
                {
                    selected_index = i;
                    continue;
                }

                if (enemies[i].health > enemies[selected_index].health)
                {
                    selected_index = i;
                }
            }

            break;
        }

        case WEAKEST:
        {
            // Find the enemy with the lowest health within range
            for (int i = 0; i < length(enemies); i++)
            {
                if (!enemy_in_range(current_tower, spot, enemies[i])) continue;

                if (selected_index == -1)
                {
                    selected_index = i;
                    continue;
                }

                if (enemies[i].health < enemies[selected_index].health)
                {
                    selected_index = i;
                }
            }

            break;
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
    if (game.selected_spot_index < 0 || game.selected_spot_index >= length(game.tower_spots)) return;

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

int get_upgrade_cost(const tower &current_tower)
{
    if (current_tower.level >= 5) return 0;

    switch(current_tower.type)
    {
    case BASIC:
        switch(current_tower.level)
        {
        case 1: return 20;
        case 2: return 30;
        case 3: return 45;
        case 4: return 60;
        }

        break;

    case FIRE:
        switch(current_tower.level)
        {
        case 1: return 30;
        case 2: return 45;
        case 3: return 65;
        case 4: return 90;
        }

        break;

    case RAPID:
        switch(current_tower.level)
        {
        case 1: return 35;
        case 2: return 50;
        case 3: return 70;
        case 4: return 95;
        }

        break;

    case HEAVY:
        switch(current_tower.level)
        {
        case 1: return 45;
        case 2: return 65;
        case 3: return 90;
        case 4: return 120;
        }

        break;
    }

    return 0;
}

bool upgrade_tower(game_data &game)
{
    // Validate the selected tower spot
    if (game.selected_spot_index < 0 || game.selected_spot_index >= length(game.tower_spots)) return false;

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

            int upgrade_cost = get_upgrade_cost(current_tower);
            int damage_increase = 0;
            double range_increase = 0;
            double attack_delay_reduction = 0;

            switch(current_tower.type)
            {
            case BASIC:
                switch(current_tower.level)
                {
                case 1:
                    damage_increase = 5;
                    break;

                case 2:
                    range_increase = 15;
                    break;

                case 3:
                    damage_increase = 10;
                    break;

                case 4:
                    damage_increase = 10;
                    range_increase = 15;
                    break;
                }

                break;

            case FIRE:
                switch(current_tower.level)
                {
                case 1:
                    damage_increase = 7;
                    break;

                case 2:
                    range_increase = 15;
                    break;

                case 3:
                    damage_increase = 10;
                    break;

                case 4:
                    damage_increase = 15;
                    range_increase = 15;
                    break;
                }

                break;

            case RAPID:
                switch(current_tower.level)
                {
                case 1:
                    attack_delay_reduction = 0.05;
                    break;

                case 2:
                    damage_increase = 3;
                    break;

                case 3:
                    attack_delay_reduction = 0.05;
                    break;

                case 4:
                    damage_increase = 4;
                    range_increase = 10;
                    break;
                }

                break;

            case HEAVY:
                switch(current_tower.level)
                {
                case 1:
                    damage_increase = 15;
                    break;

                case 2:
                    range_increase = 15;
                    break;

                case 3:
                    damage_increase = 20;
                    break;

                case 4:
                    damage_increase = 25;
                    range_increase = 15;
                    break;
                }

                break;
            }

            if (upgrade_cost == 0) return false;
            if (game.coins < upgrade_cost) return false;

            game.coins -= upgrade_cost;

            current_tower.damage += damage_increase;
            current_tower.range += range_increase;
            current_tower.attack_delay -= attack_delay_reduction;

            current_tower.level++;

            return true;
        }
    }

    return false;
}