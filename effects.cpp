#include "effects.h"

// Extend an effect duration without shortening an existing effect
static void extend_effect(
    double &end_time,
    double current_time,
    double duration)
{
    double new_end_time = current_time + duration;

    if (new_end_time > end_time)
    {
        end_time = new_end_time;
    }
}

// Apply an elemental status effect to an enemy
void apply_effect(
    enemy &current_enemy,
    effect_type type,
    double current_time,
    double duration)
{
    extend_effect(
        current_enemy.effects.end_times[type],
        current_time,
        duration);
}

// Check whether an enemy currently has a specific elemental status
bool is_effect_active(
    const enemy &current_enemy,
    effect_type type,
    double current_time)
{
    return current_time < current_enemy.effects.end_times[type];
}

// Apply or extend the movement slowdown caused by a WATER tower
void apply_wet_slow(
    enemy &current_enemy,
    double current_time,
    double duration)
{
    extend_effect(
        current_enemy.effects.wet_slow_end_time,
        current_time,
        duration);
}

// Damage an enemy and reward coins when it is defeated
void damage_enemy(
    enemy &current_enemy,
    int damage,
    int &coins)
{
    // Prevent inactive enemies from taking damage or giving rewards again
    if (!current_enemy.active || damage <= 0)
    {
        return;
    }

    current_enemy.health -= damage;

    if (current_enemy.health <= 0)
    {
        current_enemy.health = 0;
        current_enemy.active = false;

        // Each enemy rewards coins only once when defeated
        coins += current_enemy.reward;
    }
}

// Apply periodic BURNING damage every 0.5 seconds
static void update_burning(
    enemy &current_enemy,
    double current_time,
    int &coins)
{
    if (current_enemy.effects.next_burn_tick_time <= 0.0)
    {
        return;
    }

    // Process all scheduled burn ticks that have occurred
    while (
        current_enemy.active &&
        current_enemy.effects.next_burn_tick_time <= current_time &&
        current_enemy.effects.next_burn_tick_time <=
            current_enemy.effects.end_times[BURNING])
    {
        // BURNING deals 2 damage per tick for up to 2.5 seconds
        damage_enemy(current_enemy, 2, coins);

        current_enemy.effects.next_burn_tick_time += 0.5;
    }
}

// Update ongoing elemental effects for all active enemies
void update_enemy_effects(
    dynamic_array<enemy> &enemies,
    double current_time,
    int &coins)
{
    for (int i = 0; i < length(enemies); i++)
    {
        if (!enemies[i].active)
        {
            continue;
        }

        update_burning(
            enemies[i],
            current_time,
            coins);
    }
}

// Check whether an enemy can trigger another elemental reaction
bool can_trigger_reaction(
    const enemy &current_enemy,
    double current_time)
{
    return current_enemy.active &&
           current_time >=
               current_enemy.effects.reaction_cooldown_end_time;
}

// Start a cooldown to prevent repeated reactions in quick succession
void start_reaction_cooldown(
    enemy &current_enemy,
    double current_time,
    double duration)
{
    current_enemy.effects.reaction_cooldown_end_time =
        current_time + duration;
}

// Check whether two elemental statuses can react with each other
static bool has_reaction_pair(
    const enemy &current_enemy,
    effect_type first,
    effect_type second,
    double current_time)
{
    if (!can_trigger_reaction(current_enemy, current_time))
    {
        return false;
    }

    return is_effect_active(current_enemy, first, current_time) &&
           is_effect_active(current_enemy, second, current_time);
}

// Remove an elemental status after it has been used in a reaction
static void consume_effect(
    enemy &current_enemy,
    effect_type type,
    double current_time)
{
    current_enemy.effects.end_times[type] = current_time;

    // Remove the WATER slowdown when WET is consumed
    if (type == WET)
    {
        current_enemy.effects.wet_slow_end_time = current_time;
    }

    // Stop future BURNING ticks when the status is consumed
    if (type == BURNING)
    {
        current_enemy.effects.next_burn_tick_time = 0.0;
    }
}

// Consume both elemental statuses involved in a reaction
static void consume_reaction_pair(
    enemy &current_enemy,
    effect_type first,
    effect_type second,
    double current_time)
{
    consume_effect(current_enemy, first, current_time);
    consume_effect(current_enemy, second, current_time);
}

// Temporarily stop enemy movement and apply a separate stun cooldown
static void apply_stun(
    enemy &current_enemy,
    double current_time,
    double duration)
{
    // Prevent repeated stuns before the stun cooldown expires
    if (current_time < current_enemy.effects.stun_cooldown_end_time)
    {
        return;
    }

    // Boss enemies are stunned for only half the normal duration
    if (current_enemy.type == BOSS)
    {
        duration *= 0.5;
    }

    extend_effect(
        current_enemy.effects.stun_end_time,
        current_time,
        duration);

    current_enemy.effects.stun_cooldown_end_time =
        current_time + 3.0;
}

// Temporarily prevent enemy movement when MUD TRAP is triggered
static void apply_immobilize(
    enemy &current_enemy,
    double current_time,
    double duration)
{
    // Boss enemies are immobilised for only half the normal duration
    if (current_enemy.type == BOSS)
    {
        duration *= 0.5;
    }

    extend_effect(
        current_enemy.effects.immobilize_end_time,
        current_time,
        duration);
}

// Damage the triggering enemy and up to two nearby enemies
static void apply_area_damage(
    dynamic_array<enemy> &enemies,
    int enemy_index,
    int damage,
    double radius,
    int &coins)
{
    if (enemy_index < 0 || enemy_index >= length(enemies))
    {
        return;
    }

    // Save the explosion centre before applying any damage
    point_2d centre = enemies[enemy_index].position;

    // The triggering enemy is always the first target
    damage_enemy(
        enemies[enemy_index],
        damage,
        coins);

    int affected_count = 1;

    // Search for nearby active enemies within the explosion radius
    for (int i = 0; i < length(enemies); i++)
    {
        if (affected_count >= 3)
        {
            break;
        }

        if (i == enemy_index || !enemies[i].active)
        {
            continue;
        }

        double distance = point_point_distance(
            centre,
            enemies[i].position);

        if (distance <= radius)
        {
            damage_enemy(enemies[i], damage, coins);
            affected_count++;
        }
    }
}

// Detect and trigger one of the six elemental reactions
void try_trigger_reaction(
    dynamic_array<enemy> &enemies,
    int enemy_index,
    double current_time,
    int &coins)
{
    if (enemy_index < 0 || enemy_index >= length(enemies))
    {
        return;
    }

    enemy &current_enemy = enemies[enemy_index];

    // Skip inactive enemies and enemies still on reaction cooldown
    if (!can_trigger_reaction(current_enemy, current_time))
    {
        return;
    }

    bool reaction_triggered = false;

    // ELECTROSHOCK: WET + CHARGED
    // Deals bonus damage and briefly stuns the enemy
    if (has_reaction_pair(
            current_enemy,
            WET,
            CHARGED,
            current_time))
    {
        consume_reaction_pair(
            current_enemy,
            WET,
            CHARGED,
            current_time);

        damage_enemy(current_enemy, 8, coins);

        if (current_enemy.active)
        {
            apply_stun(current_enemy, current_time, 0.4);
        }

        reaction_triggered = true;
    }

    // VAPORIZE: WET + BURNING
    // Deals additional damage to a single enemy
    else if (has_reaction_pair(
                 current_enemy,
                 WET,
                 BURNING,
                 current_time))
    {
        consume_reaction_pair(
            current_enemy,
            WET,
            BURNING,
            current_time);

        damage_enemy(current_enemy, 12, coins);

        reaction_triggered = true;
    }

    // MUD TRAP: WET + CRACKED
    // Deals damage and temporarily immobilises the enemy
    else if (has_reaction_pair(
                 current_enemy,
                 WET,
                 CRACKED,
                 current_time))
    {
        consume_reaction_pair(
            current_enemy,
            WET,
            CRACKED,
            current_time);

        damage_enemy(current_enemy, 5, coins);

        if (current_enemy.active)
        {
            apply_immobilize(current_enemy, current_time, 0.6);
        }

        reaction_triggered = true;
    }

    // OVERLOAD: BURNING + CHARGED
    // Deals AOE damage within 45 pixels to up to three enemies
    else if (has_reaction_pair(
                 current_enemy,
                 BURNING,
                 CHARGED,
                 current_time))
    {
        consume_reaction_pair(
            current_enemy,
            BURNING,
            CHARGED,
            current_time);

        apply_area_damage(
            enemies,
            enemy_index,
            10,
            45.0,
            coins);

        reaction_triggered = true;
    }

    // MAGMA BURST: BURNING + CRACKED
    // Deals AOE damage within 50 pixels to up to three enemies
    else if (has_reaction_pair(
                 current_enemy,
                 BURNING,
                 CRACKED,
                 current_time))
    {
        consume_reaction_pair(
            current_enemy,
            BURNING,
            CRACKED,
            current_time);

        apply_area_damage(
            enemies,
            enemy_index,
            14,
            50.0,
            coins);

        reaction_triggered = true;
    }

    // SHATTER: CHARGED + CRACKED
    // Deals high bonus damage to a single enemy
    else if (has_reaction_pair(
                 current_enemy,
                 CHARGED,
                 CRACKED,
                 current_time))
    {
        consume_reaction_pair(
            current_enemy,
            CHARGED,
            CRACKED,
            current_time);

        damage_enemy(current_enemy, 18, coins);

        reaction_triggered = true;
    }

    // Prevent another elemental reaction for 2.5 seconds
    if (reaction_triggered)
    {
        start_reaction_cooldown(
            current_enemy,
            current_time,
            2.5);
    }
}