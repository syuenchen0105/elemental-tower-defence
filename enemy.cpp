#include "enemy.h"

void spawn_enemy(game_data &game)
{
    enemy new_enemy = {};

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

        int current = enemies[i].path_index;

        // Check if the enemy has reached the end of the path
        if (current + 1 >= length(path))
        {
            enemies[i].active = false;
            continue;
        }

        point_2d current_point = path[current];
        point_2d next_point = path[current + 1];

        // Move right
        if (current_point.x < next_point.x)
        {
            enemies[i].position.x += enemies[i].speed;

            if (enemies[i].position.x >= next_point.x)
            {
                enemies[i].position.x = next_point.x;
                enemies[i].path_index++;
            }
        }
        // Move left
        else if (current_point.x > next_point.x)
        {
            enemies[i].position.x -= enemies[i].speed;

            if (enemies[i].position.x <= next_point.x)
            {
                enemies[i].position.x = next_point.x;
                enemies[i].path_index++;
            }
        }
        // Move up
        else if (current_point.y > next_point.y)
        {
            enemies[i].position.y -= enemies[i].speed;

            if (enemies[i].position.y <= next_point.y)
            {
                enemies[i].position.y = next_point.y;
                enemies[i].path_index++;
            }
        }
        // Move down
        else if (current_point.y < next_point.y)
        {
            enemies[i].position.y += enemies[i].speed;

            if (enemies[i].position.y >= next_point.y)
            {
                enemies[i].position.y = next_point.y;
                enemies[i].path_index++;
            }
        }

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