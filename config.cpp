#include "config.h"

// Read and validate the base statistics for one tower type
static bool read_tower_stats(
    json tower_data,
    const string &type_name,
    tower_type type,
    tower_stats &stats)
{
    if (!json_has_key(tower_data, type_name))
    {
        write_line("Missing tower type: " + type_name);
        return false;
    }

    json data = json_read_object(tower_data, type_name);

    // Check required keys before reading their values
    bool valid =
        json_has_key(data, "cost") &&
        json_has_key(data, "damage") &&
        json_has_key(data, "range") &&
        json_has_key(data, "attack_delay");

    if (valid)
    {
        stats.type = type;
        stats.cost = json_read_number_as_int(data, "cost");
        stats.damage = json_read_number_as_int(data, "damage");
        stats.range = json_read_number_as_double(data, "range");
        stats.attack_delay = json_read_number_as_double(data, "attack_delay");

        // Reject invalid tower statistics
        valid =
            stats.cost > 0 &&
            stats.damage > 0 &&
            stats.range > 0.0 &&
            stats.attack_delay > 0.0;
    }

    // Release the nested JSON object
    free_json(data);

    if (!valid)
    {
        write_line("Invalid tower statistics: " + type_name);
    }

    return valid;
}

// Load the four elemental tower definitions from towers.json
void load_tower_stats(
    dynamic_array<tower_stats> &tower_definitions)
{
    json tower_data = json_from_file("towers.json");
    tower_stats stats{};

    if (read_tower_stats(tower_data, "FIRE", FIRE, stats))
    {
        add(tower_definitions, stats);
    }

    if (read_tower_stats(tower_data, "WATER", WATER, stats))
    {
        add(tower_definitions, stats);
    }

    if (read_tower_stats(tower_data, "ELECTRIC", ELECTRIC, stats))
    {
        add(tower_definitions, stats);
    }

    if (read_tower_stats(tower_data, "EARTH", EARTH, stats))
    {
        add(tower_definitions, stats);
    }

    // Release the main JSON object after loading
    free_json(tower_data);
}

// Find the base statistics of a tower type in the loaded configuration
const tower_stats *find_tower_stats(
    const game_config &config,
    tower_type type)
{
    for (int i = 0; i < length(config.tower_definitions); i++)
    {
        if (config.tower_definitions[i].type == type)
        {
            return &config.tower_definitions[i];
        }
    }

    return nullptr;
}

// Read and validate one upgrade level for a tower type
static bool read_tower_upgrade(
    json upgrade_data,
    const string &type_name,
    tower_type type,
    int level,
    tower_upgrade &upgrade)
{
    if (!json_has_key(upgrade_data, type_name))
    {
        write_line("Missing upgrades for tower: " + type_name);
        return false;
    }

    json type_data = json_read_object(upgrade_data, type_name);
    string level_name = to_string(level);

    // Check whether the requested upgrade level exists
    if (!json_has_key(type_data, level_name))
    {
        write_line(
            "Missing upgrade level " + level_name +
            " for " + type_name);

        free_json(type_data);
        return false;
    }

    json level_data = json_read_object(type_data, level_name);

    // Check all required upgrade properties
    bool valid =
        json_has_key(level_data, "cost") &&
        json_has_key(level_data, "damage_increase") &&
        json_has_key(level_data, "range_increase") &&
        json_has_key(level_data, "attack_delay_reduction");

    if (valid)
    {
        upgrade.type = type;
        upgrade.level = level;

        upgrade.cost = json_read_number_as_int(
            level_data,
            "cost");

        upgrade.damage_increase = json_read_number_as_int(
            level_data,
            "damage_increase");

        upgrade.range_increase = json_read_number_as_double(
            level_data,
            "range_increase");

        upgrade.attack_delay_reduction = json_read_number_as_double(
            level_data,
            "attack_delay_reduction");

        // An upgrade may leave some statistics unchanged
        valid =
            upgrade.cost > 0 &&
            upgrade.damage_increase >= 0 &&
            upgrade.range_increase >= 0.0 &&
            upgrade.attack_delay_reduction >= 0.0;
    }

    // Release both nested JSON objects
    free_json(level_data);
    free_json(type_data);

    if (!valid)
    {
        write_line(
            "Invalid upgrade level " + level_name +
            " for " + type_name);
    }

    return valid;
}

// Load tower upgrades for levels 2 through 5 from upgrades.json
void load_tower_upgrades(
    dynamic_array<tower_upgrade> &tower_upgrades)
{
    const int FIRST_UPGRADE_LEVEL = 2;
    const int MAX_TOWER_LEVEL = 5;

    json upgrade_data = json_from_file("upgrades.json");
    tower_upgrade upgrade{};

    // Load upgrades for all four elemental towers
    for (int level = FIRST_UPGRADE_LEVEL; level <= MAX_TOWER_LEVEL; level++)
    {
        if (read_tower_upgrade(
                upgrade_data, "FIRE", FIRE, level, upgrade))
        {
            add(tower_upgrades, upgrade);
        }

        if (read_tower_upgrade(
                upgrade_data, "WATER", WATER, level, upgrade))
        {
            add(tower_upgrades, upgrade);
        }

        if (read_tower_upgrade(
                upgrade_data, "ELECTRIC", ELECTRIC, level, upgrade))
        {
            add(tower_upgrades, upgrade);
        }

        if (read_tower_upgrade(
                upgrade_data, "EARTH", EARTH, level, upgrade))
        {
            add(tower_upgrades, upgrade);
        }
    }

    // Release the main JSON object
    free_json(upgrade_data);
}

// Find upgrade statistics for a tower type and level
const tower_upgrade *find_tower_upgrade(
    const game_config &config,
    tower_type type,
    int level)
{
    for (int i = 0; i < length(config.tower_upgrades); i++)
    {
        if (
            config.tower_upgrades[i].type == type &&
            config.tower_upgrades[i].level == level)
        {
            return &config.tower_upgrades[i];
        }
    }

    return nullptr;
}

// Read and validate the base statistics for one enemy type
static bool read_enemy_stats(
    json enemy_data,
    const string &type_name,
    enemy_type type,
    enemy_stats &stats)
{
    if (!json_has_key(enemy_data, type_name))
    {
        write_line("Missing enemy type: " + type_name);
        return false;
    }

    json data = json_read_object(enemy_data, type_name);

    // Check required keys before reading enemy statistics
    bool valid =
        json_has_key(data, "health") &&
        json_has_key(data, "speed") &&
        json_has_key(data, "reward");

    if (valid)
    {
        stats.type = type;
        stats.health = json_read_number_as_int(data, "health");
        stats.speed = json_read_number_as_double(data, "speed");
        stats.reward = json_read_number_as_int(data, "reward");

        // Validate enemy health, speed and coin reward
        valid =
            stats.health > 0 &&
            stats.speed > 0.0 &&
            stats.reward >= 0;
    }

    free_json(data);

    if (!valid)
    {
        write_line("Invalid enemy statistics: " + type_name);
    }

    return valid;
}

// Load all five enemy definitions from enemies.json
void load_enemy_stats(
    dynamic_array<enemy_stats> &enemy_definitions)
{
    json enemy_data = json_from_file("enemies.json");
    enemy_stats stats{};

    if (read_enemy_stats(enemy_data, "NORMAL", NORMAL, stats))
    {
        add(enemy_definitions, stats);
    }

    if (read_enemy_stats(enemy_data, "FAST", FAST, stats))
    {
        add(enemy_definitions, stats);
    }

    if (read_enemy_stats(enemy_data, "TANK", TANK, stats))
    {
        add(enemy_definitions, stats);
    }

    if (read_enemy_stats(enemy_data, "SWARM", SWARM, stats))
    {
        add(enemy_definitions, stats);
    }

    if (read_enemy_stats(enemy_data, "BOSS", BOSS, stats))
    {
        add(enemy_definitions, stats);
    }

    // Release the main JSON object
    free_json(enemy_data);
}

// Find the base statistics of an enemy type in the loaded configuration
const enemy_stats *find_enemy_stats(
    const game_config &config,
    enemy_type type)
{
    for (int i = 0; i < length(config.enemy_definitions); i++)
    {
        if (config.enemy_definitions[i].type == type)
        {
            return &config.enemy_definitions[i];
        }
    }

    return nullptr;
}