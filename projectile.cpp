#include "projectile.h"
#include "effects.h"

// Create a projectile that travels toward a target enemy
void fire_projectile(
    dynamic_array<projectile> &projectiles,
    const tower &current_tower,
    const tower_spot &spot,
    int target_enemy_id)
{
    projectile new_projectile{};

    new_projectile.position = spot.position;
    new_projectile.target_enemy_id = target_enemy_id;
    new_projectile.type = current_tower.type;
    new_projectile.damage = current_tower.damage;
    new_projectile.speed = 5.0;
    new_projectile.active = true;

    add(projectiles, new_projectile);
}

// Find an enemy's array index using its unique ID
static int find_enemy_index(
    const dynamic_array<enemy> &enemies,
    int enemy_id)
{
    for (int i = 0; i < length(enemies); i++)
    {
        if (enemies[i].id == enemy_id)
        {
            return i;
        }
    }

    return -1;
}

// Move a projectile toward its target and report whether it has arrived
static bool move_projectile(
    projectile &current_projectile,
    point_2d target_position)
{
    double distance = point_point_distance(
        current_projectile.position,
        target_position);

    double dx = target_position.x - current_projectile.position.x;
    double dy = target_position.y - current_projectile.position.y;

    // Move directly to the target when close enough
    if (distance <= current_projectile.speed)
    {
        current_projectile.position = target_position;
        return true;
    }

    // Move toward the target using a unit direction vector
    current_projectile.position.x +=
        (dx / distance) * current_projectile.speed;

    current_projectile.position.y +=
        (dy / distance) * current_projectile.speed;

    return false;
}

// Calculate projectile damage after applying enemy resistances
static int calculate_projectile_damage(
    const projectile &current_projectile,
    const enemy &current_enemy)
{
    int damage = current_projectile.damage;

    // TANK enemies resist 30% of EARTH projectile damage
    if (current_enemy.type == TANK && current_projectile.type == EARTH)
    {
        damage = damage * 70 / 100;
    }
    // BOSS enemies resist 20% of all direct projectile damage
    else if (current_enemy.type == BOSS)
    {
        damage = damage * 80 / 100;
    }

    // Ensure a projectile always deals at least 1 damage
    if (damage < 1)
    {
        damage = 1;
    }

    return damage;
}

// Apply the elemental status associated with a projectile's tower type
static void apply_projectile_effect(
    const projectile &current_projectile,
    enemy &current_enemy,
    double current_time)
{
    switch (current_projectile.type)
    {
    case FIRE:
        // Initialise burning damage timing only when burning is inactive
        if (!is_effect_active(current_enemy, BURNING, current_time))
        {
            current_enemy.effects.next_burn_tick_time =
                current_time + 0.5;
        }

        apply_effect(
            current_enemy,
            BURNING,
            current_time,
            2.5);

        break;

    case WATER:
        // Apply WET status and its separate movement slowdown
        apply_effect(
            current_enemy,
            WET,
            current_time,
            3.0);

        apply_wet_slow(
            current_enemy,
            current_time,
            3.0);

        break;

    case ELECTRIC:
        apply_effect(
            current_enemy,
            CHARGED,
            current_time,
            2.5);

        break;

    case EARTH:
        apply_effect(
            current_enemy,
            CRACKED,
            current_time,
            2.5);

        break;
    }
}

// Apply projectile damage and its elemental status if the enemy survives
static void handle_projectile_hit(
    projectile &current_projectile,
    enemy &current_enemy,
    int &coins,
    double current_time)
{
    // Deactivate the projectile after reaching its target
    current_projectile.active = false;

    int damage = calculate_projectile_damage(
        current_projectile,
        current_enemy);

    damage_enemy(
        current_enemy,
        damage,
        coins);

    if (current_enemy.active)
    {
        apply_projectile_effect(
            current_projectile,
            current_enemy,
            current_time);
    }
}

// Update active projectiles and handle hits and elemental reactions
void update_projectiles(
    dynamic_array<projectile> &projectiles,
    dynamic_array<enemy> &enemies,
    int &coins,
    double current_time)
{
    for (int i = 0; i < length(projectiles); i++)
    {
        if (!projectiles[i].active)
        {
            continue;
        }

        // Find the projectile's target enemy
        int target_index = find_enemy_index(
            enemies,
            projectiles[i].target_enemy_id);

        // Deactivate the projectile if its target is missing or inactive
        if (target_index == -1 || !enemies[target_index].active)
        {
            projectiles[i].active = false;
            continue;
        }

        // Move the projectile and process its hit when it reaches the enemy
        if (move_projectile(
                projectiles[i],
                enemies[target_index].position))
        {
            handle_projectile_hit(
                projectiles[i],
                enemies[target_index],
                coins,
                current_time);

            // Check for elemental reactions after applying the status
            if (enemies[target_index].active)
            {
                try_trigger_reaction(
                    enemies,
                    target_index,
                    current_time,
                    coins);
            }
        }
    }
}

// Remove inactive projectiles by iterating backward through the array
void remove_inactive_projectiles(
    dynamic_array<projectile> &projectiles)
{
    for (int i = length(projectiles) - 1; i >= 0; i--)
    {
        if (!projectiles[i].active)
        {
            remove(projectiles, i);
        }
    }
}