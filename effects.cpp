#include "effects.h"

// Read and validate the statistics for one elemental reaction
static bool read_reaction_stats(
    json reactions_data,
    const string &name,
    reaction_stats &result)
{
    if (!json_has_key(reactions_data, name))
    {
        return false;
    }

    json reaction_data = json_read_object(
        reactions_data,
        name);

    // Check that all required reaction properties are present
    if (
        !json_has_key(reaction_data, "damage") ||
        !json_has_key(reaction_data, "radius") ||
        !json_has_key(reaction_data, "control_duration") ||
        !json_has_key(reaction_data, "cooldown"))
    {
        free_json(reaction_data);
        return false;
    }

    // Read the reaction properties from JSON
    result.damage = json_read_number_as_int(
        reaction_data,
        "damage");

    result.radius = json_read_number_as_double(
        reaction_data,
        "radius");

    result.control_duration = json_read_number_as_double(
        reaction_data,
        "control_duration");

    result.cooldown = json_read_number_as_double(
        reaction_data,
        "cooldown");

    free_json(reaction_data);

    // Reject negative damage, ranges and durations
    return result.damage >= 0 &&
           result.radius >= 0.0 &&
           result.control_duration >= 0.0 &&
           result.cooldown >= 0.0;
}

// Load all six reactions and shared settings from reactions.json
bool load_reaction_config(
    reaction_config &settings)
{
    json root = json_from_file("reactions.json");

    // Verify that the required top-level sections exist
    if (
        !json_has_key(root, "settings") ||
        !json_has_key(root, "reactions"))
    {
        write_line("Error: reactions.json is missing sections.");
        free_json(root);
        return false;
    }

    json general_data = json_read_object(
        root,
        "settings");

    json reactions_data = json_read_object(
        root,
        "reactions");

    // Check that the shared configuration is complete
    if (
        !json_has_key(general_data, "max_aoe_targets") ||
        !json_has_key(general_data, "stun_cooldown") ||
        !json_has_key(general_data, "boss_control_multiplier"))
    {
        write_line("Error: Missing shared reaction settings.");

        free_json(general_data);
        free_json(reactions_data);
        free_json(root);

        return false;
    }

    reaction_config loaded{};

    loaded.max_aoe_targets = json_read_number_as_int(
        general_data,
        "max_aoe_targets");

    loaded.stun_cooldown = json_read_number_as_double(
        general_data,
        "stun_cooldown");

    loaded.boss_control_multiplier = json_read_number_as_double(
        general_data,
        "boss_control_multiplier");

    // Load each reaction into its corresponding fixed-array position
    bool valid =
        read_reaction_stats(
            reactions_data,
            "ELECTROSHOCK",
            loaded.definitions[ELECTROSHOCK]) &&
        read_reaction_stats(
            reactions_data,
            "VAPORIZE",
            loaded.definitions[VAPORIZE]) &&
        read_reaction_stats(
            reactions_data,
            "MUD_TRAP",
            loaded.definitions[MUD_TRAP]) &&
        read_reaction_stats(
            reactions_data,
            "OVERLOAD",
            loaded.definitions[OVERLOAD]) &&
        read_reaction_stats(
            reactions_data,
            "MAGMA_BURST",
            loaded.definitions[MAGMA_BURST]) &&
        read_reaction_stats(
            reactions_data,
            "SHATTER",
            loaded.definitions[SHATTER]);

    // Validate shared values before accepting the configuration
    if (
        loaded.max_aoe_targets < 1 ||
        loaded.stun_cooldown < 0.0 ||
        loaded.boss_control_multiplier < 0.0 ||
        loaded.boss_control_multiplier > 1.0)
    {
        valid = false;
    }

    if (valid)
    {
        settings = loaded;
    }
    else
    {
        write_line("Error: Invalid reaction values in reactions.json.");
    }

    // Release all JSON resources after loading
    free_json(general_data);
    free_json(reactions_data);
    free_json(root);

    return valid;
}

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

// Check whether an enemy has an active elemental status
bool is_effect_active(
    const enemy &current_enemy,
    effect_type type,
    double current_time)
{
    return current_time < current_enemy.effects.end_times[type];
}

// Apply or extend the movement slowdown caused by WATER
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

// Damage an enemy and award coins when it is defeated
void damage_enemy(
    enemy &current_enemy,
    int damage,
    int &coins)
{
    // Prevent duplicate rewards and invalid damage
    if (!current_enemy.active || damage <= 0)
    {
        return;
    }

    current_enemy.health -= damage;

    if (current_enemy.health <= 0)
    {
        current_enemy.health = 0;
        current_enemy.active = false;
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

    // Process all burning ticks that have occurred
    while (
        current_enemy.active &&
        current_enemy.effects.next_burn_tick_time <= current_time &&
        current_enemy.effects.next_burn_tick_time <=
            current_enemy.effects.end_times[BURNING])
    {
        damage_enemy(current_enemy, 2, coins);

        current_enemy.effects.next_burn_tick_time += 0.5;
    }
}

// Update ongoing elemental effects for every active enemy
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

// Check whether an enemy is allowed to trigger another reaction
bool can_trigger_reaction(
    const enemy &current_enemy,
    double current_time)
{
    return current_enemy.active &&
           current_time >=
               current_enemy.effects.reaction_cooldown_end_time;
}

// Start a cooldown after a successful elemental reaction
void start_reaction_cooldown(
    enemy &current_enemy,
    double current_time,
    double duration)
{
    current_enemy.effects.reaction_cooldown_end_time =
        current_time + duration;
}

// Check whether two elemental statuses can react
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

// Remove a status once it has been consumed by a reaction
static void consume_effect(
    enemy &current_enemy,
    effect_type type,
    double current_time)
{
    current_enemy.effects.end_times[type] = current_time;

    // Remove the movement slowdown when WET is consumed
    if (type == WET)
    {
        current_enemy.effects.wet_slow_end_time = current_time;
    }

    // Prevent further burning ticks when BURNING is consumed
    if (type == BURNING)
    {
        current_enemy.effects.next_burn_tick_time = 0.0;
    }
}

// Consume the two elemental statuses involved in a reaction
static void consume_reaction_pair(
    enemy &current_enemy,
    effect_type first,
    effect_type second,
    double current_time)
{
    consume_effect(current_enemy, first, current_time);
    consume_effect(current_enemy, second, current_time);
}

// Detect which reaction should occur based on active statuses
static reaction_type find_reaction(
    const enemy &current_enemy,
    double current_time)
{
    // Preserve the original priority of the six reactions
    if (has_reaction_pair(
            current_enemy, WET, CHARGED, current_time))
    {
        return ELECTROSHOCK;
    }

    if (has_reaction_pair(
            current_enemy, WET, BURNING, current_time))
    {
        return VAPORIZE;
    }

    if (has_reaction_pair(
            current_enemy, WET, CRACKED, current_time))
    {
        return MUD_TRAP;
    }

    if (has_reaction_pair(
            current_enemy, BURNING, CHARGED, current_time))
    {
        return OVERLOAD;
    }

    if (has_reaction_pair(
            current_enemy, BURNING, CRACKED, current_time))
    {
        return MAGMA_BURST;
    }

    if (has_reaction_pair(
            current_enemy, CHARGED, CRACKED, current_time))
    {
        return SHATTER;
    }

    // REACTION_COUNT represents no matching reaction
    return REACTION_COUNT;
}

// Consume the correct elemental pair for the detected reaction
static void consume_reaction_effects(
    enemy &current_enemy,
    reaction_type type,
    double current_time)
{
    switch (type)
    {
    case ELECTROSHOCK:
        consume_reaction_pair(
            current_enemy, WET, CHARGED, current_time);
        break;

    case VAPORIZE:
        consume_reaction_pair(
            current_enemy, WET, BURNING, current_time);
        break;

    case MUD_TRAP:
        consume_reaction_pair(
            current_enemy, WET, CRACKED, current_time);
        break;

    case OVERLOAD:
        consume_reaction_pair(
            current_enemy, BURNING, CHARGED, current_time);
        break;

    case MAGMA_BURST:
        consume_reaction_pair(
            current_enemy, BURNING, CRACKED, current_time);
        break;

    case SHATTER:
        consume_reaction_pair(
            current_enemy, CHARGED, CRACKED, current_time);
        break;

    case REACTION_COUNT:
        break;
    }
}

// Apply a temporary stun with boss resistance and a stun cooldown
static void apply_stun(
    enemy &current_enemy,
    double current_time,
    double duration,
    const reaction_config &settings)
{
    // Prevent repeatedly stunning the same enemy
    if (current_time < current_enemy.effects.stun_cooldown_end_time)
    {
        return;
    }

    // Bosses receive a shorter stun duration
    if (current_enemy.type == BOSS)
    {
        duration *= settings.boss_control_multiplier;
    }

    extend_effect(
        current_enemy.effects.stun_end_time,
        current_time,
        duration);

    current_enemy.effects.stun_cooldown_end_time =
        current_time + settings.stun_cooldown;
}

// Apply temporary immobilisation, with shorter duration for bosses
static void apply_immobilize(
    enemy &current_enemy,
    double current_time,
    double duration,
    const reaction_config &settings)
{
    if (current_enemy.type == BOSS)
    {
        duration *= settings.boss_control_multiplier;
    }

    extend_effect(
        current_enemy.effects.immobilize_end_time,
        current_time,
        duration);
}

// Damage the triggering enemy and a limited number of nearby enemies
static void apply_area_damage(
    dynamic_array<enemy> &enemies,
    int enemy_index,
    int damage,
    double radius,
    int max_targets,
    int &coins)
{
    if (enemy_index < 0 || enemy_index >= length(enemies))
    {
        return;
    }

    // Save the explosion centre before applying damage
    point_2d centre = enemies[enemy_index].position;

    // The triggering enemy always counts as the first target
    damage_enemy(
        enemies[enemy_index],
        damage,
        coins);

    int affected_count = 1;

    // Search for nearby enemies inside the reaction radius
    for (int i = 0; i < length(enemies); i++)
    {
        if (affected_count >= max_targets)
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

// Detect and execute a reaction using the loaded JSON statistics
void try_trigger_reaction(
    dynamic_array<enemy> &enemies,
    int enemy_index,
    double current_time,
    int &coins,
    const reaction_config &settings)
{
    if (enemy_index < 0 || enemy_index >= length(enemies))
    {
        return;
    }

    enemy &current_enemy = enemies[enemy_index];

    // Determine whether the enemy has a valid elemental pair
    reaction_type type = find_reaction(
        current_enemy,
        current_time);

    if (type == REACTION_COUNT)
    {
        return;
    }

    // Retrieve the reaction's damage and durations from configuration
    const reaction_stats &stats = settings.definitions[type];

    // Remove the two elemental effects consumed by the reaction
    consume_reaction_effects(
        current_enemy,
        type,
        current_time);

    // Apply the appropriate damage or movement restriction
    switch (type)
    {
    case ELECTROSHOCK:
        damage_enemy(current_enemy, stats.damage, coins);

        if (current_enemy.active)
        {
            apply_stun(
                current_enemy,
                current_time,
                stats.control_duration,
                settings);
        }
        break;

    case VAPORIZE:
        damage_enemy(current_enemy, stats.damage, coins);
        break;

    case MUD_TRAP:
        damage_enemy(current_enemy, stats.damage, coins);

        if (current_enemy.active)
        {
            apply_immobilize(
                current_enemy,
                current_time,
                stats.control_duration,
                settings);
        }
        break;

    case OVERLOAD:
    case MAGMA_BURST:
        apply_area_damage(
            enemies,
            enemy_index,
            stats.damage,
            stats.radius,
            settings.max_aoe_targets,
            coins);
        break;

    case SHATTER:
        damage_enemy(current_enemy, stats.damage, coins);
        break;

    case REACTION_COUNT:
        break;
    }

    // Apply the reaction-specific cooldown loaded from JSON
    start_reaction_cooldown(
        current_enemy,
        current_time,
        stats.cooldown);
}