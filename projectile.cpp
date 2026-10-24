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

int find_enemy_index(
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

bool move_projectile(
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

void update_projectiles(
    dynamic_array<projectile> &projectiles,
    dynamic_array<enemy> &enemies,
    int &coins
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
            // Apply damage and deactivate the projectile
            enemies[target_index].health -= projectiles[i].damage;
            projectiles[i].active = false;

            if (enemies[target_index].health <= 0)
            {
                enemies[target_index].active = false;
                coins += enemies[target_index].reward;
            }
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