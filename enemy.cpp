#include "enemy.h"
#include "config.h"

// Shared enemy spawning and movement constants
const int BOSS_WAVE_INTERVAL = 5;
const double HEALTH_INCREASE_PER_WAVE = 0.12;
const double WET_SPEED_MULTIPLIER = 0.75;

// Determine which enemy type should spawn based on the wave and spawn index
enemy_type get_wave_enemy_type(
    int wave,
    int spawn_index,
    int enemies_per_wave)
{
    // Spawn a boss as the final enemy of every fifth wave
    if (
        wave % BOSS_WAVE_INTERVAL == 0 &&
        spawn_index == enemies_per_wave - 1)
    {
        return BOSS;
    }

    // Introduce SWARM enemies starting from wave 4
    if (wave >= 4 && spawn_index % 6 >= 3)
    {
        return SWARM;
    }

    // Introduce TANK enemies starting from wave 3
    if (wave >= 3 && spawn_index % 3 == 2)
    {
        return TANK;
    }

    // Introduce FAST enemies starting from wave 2
    if (wave >= 2 && spawn_index % 2 == 1)
    {
        return FAST;
    }

    // NORMAL enemies are available from the first wave
    return NORMAL;
}

// Create a new enemy using its JSON statistics and current wave scaling
void spawn_enemy(
    game_data &game,
    enemy_type type)
{
    // Find the statistics for the selected enemy type
    const enemy_stats *stats = find_enemy_stats(
        game.config,
        type);

    if (stats == nullptr)
    {
        return;
    }

    // An enemy cannot spawn without a valid path
    if (length(game.path) == 0)
    {
        return;
    }

    enemy new_enemy{};

    // Assign a unique ID and the starting path position
    new_enemy.id = game.next_enemy_id++;
    new_enemy.type = type;
    new_enemy.position = game.path[0];
    new_enemy.path_index = 0;

    // Increase enemy health by 12% for every wave after wave 1
    double health_multiplier =
        1.0 + HEALTH_INCREASE_PER_WAVE * (game.wave - 1);

    // Convert the scaled health to a whole number
    new_enemy.max_health = stats->health * health_multiplier;

    new_enemy.health = new_enemy.max_health;
    new_enemy.speed = stats->speed;
    new_enemy.reward = stats->reward;
    new_enemy.active = true;

    // Add the new enemy to the dynamic array
    add(game.enemies, new_enemy);
}

// Move an enemy toward its next waypoint while applying movement effects
static void move_enemy(
    enemy &current_enemy,
    const dynamic_array<point_2d> &path,
    double current_time)
{
    // Prevent movement while the enemy is stunned
    if (current_time < current_enemy.effects.stun_end_time)
    {
        return;
    }

    // Prevent movement while the enemy is immobilised
    if (current_time < current_enemy.effects.immobilize_end_time)
    {
        return;
    }

    // Find the next waypoint along the enemy path
    int next_index = current_enemy.path_index + 1;

    if (next_index >= length(path))
    {
        return;
    }

    point_2d next_point = path[next_index];

    double movement_speed = current_enemy.speed;

    // Reduce movement speed by 25% while affected by WATER slowdown
    if (current_time < current_enemy.effects.wet_slow_end_time)
    {
        movement_speed *= WET_SPEED_MULTIPLIER;
    }

    // Calculate the distance to the next waypoint
    double distance = point_point_distance(
        current_enemy.position,
        next_point);

    // Move directly to the waypoint when close enough
    if (distance <= movement_speed)
    {
        current_enemy.position = next_point;
        current_enemy.path_index++;
        return;
    }

    // Calculate the direction toward the next waypoint
    double dx = next_point.x - current_enemy.position.x;
    double dy = next_point.y - current_enemy.position.y;

    // Move the enemy in the direction of the next waypoint
    current_enemy.position.x +=
        (dx / distance) * movement_speed;

    current_enemy.position.y +=
        (dy / distance) * movement_speed;
}

// Update enemy movement and reduce lives when enemies reach the exit
void update_enemies(
    dynamic_array<enemy> &enemies,
    const dynamic_array<point_2d> &path,
    int &lives,
    double current_time)
{
    for (int i = 0; i < length(enemies); i++)
    {
        // Stop processing enemies when the game is over
        if (lives <= 0)
        {
            break;
        }

        // Ignore enemies that have already been defeated or escaped
        if (!enemies[i].active)
        {
            continue;
        }

        // Move the enemy along its path
        move_enemy(
            enemies[i],
            path,
            current_time);

        // Reduce player lives when an enemy reaches the final waypoint
        if (enemies[i].path_index == length(path) - 1)
        {
            lives--;
            enemies[i].active = false;
        }
    }
}

// Remove inactive enemies by iterating backward through the dynamic array
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