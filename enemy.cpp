#include "enemy.h"

void spawn_enemy(game_data &game)
{
    enemy new_enemy{};

    new_enemy.id = game.next_enemy_id;
    game.next_enemy_id++;

    new_enemy.type = NORMAL;
    new_enemy.position = game.path[0];
    new_enemy.path_index = 0;

    new_enemy.health = 100;
    new_enemy.max_health = 100;
    new_enemy.speed = 2.0;
    new_enemy.reward = 10;
    new_enemy.active = true;

    add(game.enemies, new_enemy);
}

void move_enemy(
    enemy &current_enemy,
    const dynamic_array<point_2d> &path
)
{
    int next_index = current_enemy.path_index + 1;

    if (next_index >= length(path)) return;

    point_2d next_point = path[next_index];

    double distance = point_point_distance(
        current_enemy.position,
        next_point
    );

    // Move to the next waypoint when close enough
    if (distance <= current_enemy.speed)
    {
        current_enemy.position = next_point;
        current_enemy.path_index++;
        return;
    }

    double dx = next_point.x - current_enemy.position.x;
    double dy = next_point.y - current_enemy.position.y;

    current_enemy.position.x += (dx / distance) * current_enemy.speed;
    current_enemy.position.y += (dy / distance) * current_enemy.speed;
}

void update_enemies(
    dynamic_array<enemy> &enemies,
    const dynamic_array<point_2d> &path,
    int &lives
)
{
    for (int i = 0; i < length(enemies); i++)
    {
        if (lives <= 0) break;
        if (!enemies[i].active) continue;

        move_enemy(enemies[i], path);

        // Deduct one life when the enemy reaches the base
        if (enemies[i].path_index == length(path) - 1)
        {
            lives--;
            enemies[i].active = false;
        }
    }
}

void remove_inactive_enemies(dynamic_array<enemy> &enemies)
{
    int i = 0;

    while (i < length(enemies))
    {
        if (!enemies[i].active)
        {
            remove(enemies, i);
        }
        else
        {
            // Only increment if no enemy was removed
            i++;
        }
    }
}