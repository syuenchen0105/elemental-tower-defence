#include "effects.h"
#include "projectile.h"

void fire_projectile(
    dynamic_array<projectile> &projectiles,
    const tower &current_tower,
    const tower_spot &spot,
    int target_enemy_id
)
{
    // Create a projectile at the tower's position
    projectile new_projectile{};

    new_projectile.position = spot.position;
    new_projectile.target_enemy_id = target_enemy_id;
    new_projectile.type = current_tower.type;
    new_projectile.damage = current_tower.damage;
    new_projectile.speed = 5.0;
    new_projectile.active = true;

    add(projectiles, new_projectile);
}

static int find_enemy_index(
    const dynamic_array<enemy> &enemies,
    int enemy_id
)
{
    // Find the enemy with the matching ID
    for (int i = 0; i < length(enemies); i++)
    {
        if (enemies[i].id == enemy_id)
        {
            return i;
        }
    }

    return -1;
}

static bool move_projectile(
    projectile &current_projectile,
    point_2d target_position
)
{
    // Calculate the distance and direction to the target
    double distance = point_point_distance(
        current_projectile.position,
        target_position
    );

    double dx = target_position.x - current_projectile.position.x;
    double dy = target_position.y - current_projectile.position.y;

    // Move the projectile directly to its target when close enough
    if (distance <= current_projectile.speed)
    {
        current_projectile.position = target_position;
        return true;
    }

    // Move towards the target using a unit vector
    current_projectile.position.x += (dx / distance) * current_projectile.speed;
    current_projectile.position.y += (dy / distance) * current_projectile.speed;

    return false;
}

static void apply_projectile_effect(
    const projectile &current_projectile,
    enemy &current_enemy,
    double current_time
)
{
    switch (current_projectile.type)
    {
        case FIRE:
            if (!is_effect_active(current_enemy, BURNING, current_time))
            {
                current_enemy.effects.next_burn_tick_time = current_time + 0.5;
            }

            apply_effect(current_enemy, BURNING, current_time, 2.5);
            break;

        
        case WATER:
            apply_effect(current_enemy, WET, current_time, 3.0);
            break;
        
        case ELECTRIC:
            apply_effect(current_enemy, CHARGED, current_time, 2.5);
            break;

        case EARTH:
            apply_effect(current_enemy, CRACKED, current_time, 2.5);
            break;
    }
}

static void handle_projectile_hit(
    projectile &current_projectile,
    enemy &current_enemy,
    int &coins,
    double current_time
)
{
    // Apply damage and deactivate the projectile
    current_enemy.health -= current_projectile.damage;
    current_projectile.active = false;

    if (current_enemy.health <= 0)
    {
        current_enemy.active = false;
        coins += current_enemy.reward;
    }

    if (current_enemy.active)
    {
        apply_projectile_effect(
            current_projectile,
            current_enemy,
            current_time
        );
    }
}

void update_projectiles(
    dynamic_array<projectile> &projectiles,
    dynamic_array<enemy> &enemies,
    int &coins,
    double current_time
)
{
    for (int i = 0; i < length(projectiles); i++)
    {
        if (!projectiles[i].active) continue;

        // Find the projectile's target enemy
        int target_index = find_enemy_index(
            enemies,
            projectiles[i].target_enemy_id
        );

        // Deactivate the projectile if its target is missing or inactive
        if (target_index == -1 || !enemies[target_index].active)
        {
            projectiles[i].active = false;
            continue;
        }

        if (move_projectile(projectiles[i], enemies[target_index].position))
        {
            handle_projectile_hit(
                projectiles[i],
                enemies[target_index],
                coins,
                current_time
            );
        }
    }
}

void remove_inactive_projectiles(dynamic_array<projectile> &projectiles)
{
    int i = 0;

    while (i < length(projectiles))
    {
        if (!projectiles[i].active)
        {
            remove(projectiles, i);
        }
        else
        {
            i++;
        }
    }
}