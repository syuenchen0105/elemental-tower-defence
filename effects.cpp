#include "effects.h"

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

bool is_effect_active(
    const enemy &current_enemy,
    effect_type type,
    double current_time)
{
    return current_time < current_enemy.effects.end_times[type];
}

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

void damage_enemy(
    enemy &current_enemy,
    int damage,
    int &coins)
{
    if (!current_enemy.active || damage <= 0)
    {
        return;
    }

    current_enemy.health -= damage;

    if (current_enemy.health <= 0)
    {
        current_enemy.active = false;
        coins += current_enemy.reward;
    }
}

static void update_burning(
    enemy &current_enemy,
    double current_time,
    int &coins)
{
    while (
        current_enemy.active &&
        current_enemy.effects.next_burn_tick_time > 0.0 &&
        current_enemy.effects.next_burn_tick_time <= current_time &&
        current_enemy.effects.next_burn_tick_time <=
            current_enemy.effects.end_times[BURNING])
    {
        damage_enemy(current_enemy, 2, coins);
        current_enemy.effects.next_burn_tick_time += 0.5;
    }
}

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

        update_burning(enemies[i], current_time, coins);
    }
}

bool can_trigger_reaction(
    const enemy &current_enemy,
    double current_time)
{
    return current_enemy.active &&
           current_time >= current_enemy.effects.reaction_cooldown_end_time;
}

void start_reaction_cooldown(
    enemy &current_enemy,
    double current_time,
    double duration)
{
    current_enemy.effects.reaction_cooldown_end_time =
        current_time + duration;
}

static bool has_reaction_pair(
    const enemy &current_enemy,
    effect_type first,
    effect_type second,
    double current_time)
{
    return can_trigger_reaction(current_enemy, current_time) &&
           is_effect_active(current_enemy, first, current_time) &&
           is_effect_active(current_enemy, second, current_time);
}

static void consume_effect(
    enemy &current_enemy,
    effect_type type,
    double current_time)
{
    current_enemy.effects.end_times[type] = current_time;

    if (type == WET)
    {
        current_enemy.effects.wet_slow_end_time = current_time;
    }
    else if (type == BURNING)
    {
        current_enemy.effects.next_burn_tick_time = 0.0;
    }
}

static void consume_reaction_pair(
    enemy &current_enemy,
    effect_type first,
    effect_type second,
    double current_time)
{
    consume_effect(current_enemy, first, current_time);
    consume_effect(current_enemy, second, current_time);
}

static void apply_stun(
    enemy &current_enemy,
    double current_time,
    double duration)
{
    if (current_time < current_enemy.effects.stun_cooldown_end_time)
    {
        return;
    }

    if (current_enemy.type == BOSS)
    {
        duration *= 0.5;
    }

    extend_effect(
        current_enemy.effects.stun_end_time,
        current_time,
        duration);

    current_enemy.effects.stun_cooldown_end_time = current_time + 3.0;
}

static void apply_immobilize(
    enemy &current_enemy,
    double current_time,
    double duration)
{
    if (current_enemy.type == BOSS)
    {
        duration *= 0.5;
    }

    extend_effect(
        current_enemy.effects.immobilize_end_time,
        current_time,
        duration);
}

static void apply_area_damage(
    dynamic_array<enemy> &enemies,
    int enemy_index,
    int damage,
    double radius,
    int &coins)
{
    point_2d center = enemies[enemy_index].position;

    // The triggering enemy counts as the first target
    damage_enemy(enemies[enemy_index], damage, coins);

    int targets_hit = 1;

    for (int i = 0; i < length(enemies) && targets_hit < 3; i++)
    {
        if (i == enemy_index || !enemies[i].active)
        {
            continue;
        }

        if (point_point_distance(center, enemies[i].position) <= radius)
        {
            damage_enemy(enemies[i], damage, coins);
            targets_hit++;
        }
    }
}

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

    if (!can_trigger_reaction(current_enemy, current_time))
    {
        return;
    }

    if (has_reaction_pair(current_enemy, WET, CHARGED, current_time))
    {
        // ELECTROSHOCK
        consume_reaction_pair(
            current_enemy,
            WET,
            CHARGED,
            current_time);

        apply_stun(current_enemy, current_time, 0.4);
        damage_enemy(current_enemy, 8, coins);
    }
    else if (has_reaction_pair(current_enemy, WET, BURNING, current_time))
    {
        // VAPORIZE
        consume_reaction_pair(
            current_enemy,
            WET,
            BURNING,
            current_time);

        damage_enemy(current_enemy, 12, coins);
    }
    else if (has_reaction_pair(current_enemy, WET, CRACKED, current_time))
    {
        // MUD TRAP
        consume_reaction_pair(
            current_enemy,
            WET,
            CRACKED,
            current_time);

        apply_immobilize(current_enemy, current_time, 0.6);
        damage_enemy(current_enemy, 5, coins);
    }
    else if (has_reaction_pair(current_enemy, BURNING, CHARGED, current_time))
    {
        // OVERLOAD
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
    }
    else if (has_reaction_pair(current_enemy, BURNING, CRACKED, current_time))
    {
        // MAGMA BURST
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
    }
    else if (has_reaction_pair(current_enemy, CHARGED, CRACKED, current_time))
    {
        // SHATTER
        consume_reaction_pair(
            current_enemy,
            CHARGED,
            CRACKED,
            current_time);

        damage_enemy(current_enemy, 18, coins);
    }
    else
    {
        return;
    }

    start_reaction_cooldown(
        current_enemy,
        current_time,
        2.5);
}