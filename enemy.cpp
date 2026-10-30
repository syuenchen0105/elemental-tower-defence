#include "enemy.h"
#include "config.h"

enemy_type get_wave_enemy_type(
    int wave,
    int spawn_index,
    int enemies_per_wave)
{
    // Spawn a boss at the end of every fifth wave
    if (wave % 5 == 0 && spawn_index == enemies_per_wave - 1)
    {
        return BOSS;
    }

    if (wave >= 4 && spawn_index % 6 >= 3)
    {
        return SWARM;
    }

    if (wave >= 3 && spawn_index % 3 == 2)
    {
        return TANK;
    }

    if (wave >= 2 && spawn_index % 2 == 1)
    {
        return FAST;
    }

    return NORMAL;
}

void spawn_enemy(
    game_data &game,
    enemy_type type)
{
    const enemy_stats *stats = find_enemy_stats(game.config, type);

    if (stats == nullptr)
    {
        return;
    }

    if (length(game.path) == 0)
    {
        return;
    }

    enemy new_enemy{};

    new_enemy.id = game.next_enemy_id++;
    new_enemy.type = type;
    new_enemy.position = game.path[0];
    new_enemy.path_index = 0;

    // Increase enemy health by 12% per wave
    double health_multiplier = 1.0 + 0.12 * (game.wave - 1);

    new_enemy.max_health = static_cast<int>(
        stats->health * health_multiplier);

    new_enemy.health = new_enemy.max_health;
    new_enemy.speed = stats->speed;
    new_enemy.reward = stats->reward;
    new_enemy.active = true;

    add(game.enemies, new_enemy);
}

static void move_enemy(
    enemy &current_enemy,
    const dynamic_array<point_2d> &path,
    double current_time)
{
    // Prevent movement while stunned
    if (current_time < current_enemy.effects.stun_end_time)
    {
        return;
    }

    // Prevent movement while immobilised
    if (current_time < current_enemy.effects.immobilize_end_time)
    {
        return;
    }

    int next_index = current_enemy.path_index + 1;

    if (next_index >= length(path))
    {
        return;
    }

    point_2d next_point = path[next_index];

    double movement_speed = current_enemy.speed;

    // Reduce movement speed by 25% while slowed
    if (current_time < current_enemy.effects.wet_slow_end_time)
    {
        movement_speed *= 0.75;
    }

    double distance = point_point_distance(
        current_enemy.position,
        next_point);

    if (distance <= movement_speed)
    {
        current_enemy.position = next_point;
        current_enemy.path_index++;
        return;
    }

    double dx = next_point.x - current_enemy.position.x;
    double dy = next_point.y - current_enemy.position.y;

    current_enemy.position.x +=
        (dx / distance) * movement_speed;

    current_enemy.position.y +=
        (dy / distance) * movement_speed;
}

void update_enemies(
    dynamic_array<enemy> &enemies,
    const dynamic_array<point_2d> &path,
    int &lives,
    double current_time)
{
    for (int i = 0; i < length(enemies); i++)
    {
        if (lives <= 0)
        {
            break;
        }

        if (!enemies[i].active)
        {
            continue;
        }

        move_enemy(enemies[i], path, current_time);

        // Reduce lives when an enemy reaches the end
        if (enemies[i].path_index == length(path) - 1)
        {
            lives--;
            enemies[i].active = false;
        }
    }
}

void remove_inactive_enemies(
    dynamic_array<enemy> &enemies)
{
    for (int i = length(enemies) - 1; i >= 0; i--)
    {
        if (!enemies[i].active)
        {
            remove(enemies, i);
        }
    }
}