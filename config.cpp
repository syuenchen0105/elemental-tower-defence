#include "config.h"

static tower_stats read_tower_stats(
    json tower_data,
    const string &type_name,
    tower_type type
)
{
    tower_stats stats{};

    json data = json_read_object(tower_data, type_name);

    stats.type = type;
    stats.cost = json_read_number_as_int(data, "cost");
    stats.damage = json_read_number_as_int(data, "damage");
    stats.range = json_read_number_as_double(data, "range");
    stats.attack_delay = json_read_number_as_double(data, "attack_delay");

    return stats;
}

static tower_upgrade read_tower_upgrade(
    json upgrade_data,
    const string &type_name,
    tower_type type,
    int level
)
{
    tower_upgrade upgrade{};

    json type_data = json_read_object(upgrade_data, type_name);
    json level_data = json_read_object(type_data, to_string(level));

    upgrade.type = type;
    upgrade.level = level;
    upgrade.cost = json_read_number_as_int(level_data, "cost");
    upgrade.damage_increase = json_read_number_as_int(level_data, "damage_increase");
    upgrade.range_increase = json_read_number_as_double(level_data, "range_increase");
    upgrade.attack_delay_reduction = json_read_number_as_double(level_data, "attack_delay_reduction");

    return upgrade;
}

void load_tower_stats(dynamic_array<tower_stats> &tower_definitions)
{
    json tower_data = json_from_file("towers.json");

    add(tower_definitions, read_tower_stats(tower_data, "FIRE", FIRE));
    add(tower_definitions, read_tower_stats(tower_data, "WATER", WATER));
    add(tower_definitions, read_tower_stats(tower_data, "ELECTRIC", ELECTRIC));
    add(tower_definitions, read_tower_stats(tower_data, "EARTH", EARTH));

    free_json(tower_data);
}

void load_tower_upgrades(
    dynamic_array<tower_upgrade> &tower_upgrades
)
{
    json upgrade_data = json_from_file("upgrades.json");

    for (int i = 2; i <= 5; i++)
    {
        add(tower_upgrades, read_tower_upgrade(upgrade_data, "FIRE", FIRE, i));
        add(tower_upgrades, read_tower_upgrade(upgrade_data, "WATER", WATER, i));
        add(tower_upgrades, read_tower_upgrade(upgrade_data, "ELECTRIC", ELECTRIC, i));
        add(tower_upgrades, read_tower_upgrade(upgrade_data, "EARTH", EARTH, i));
    }

    free_json(upgrade_data);
}

const tower_stats *find_tower_stats(
    const game_config &config,
    tower_type type
)
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

const tower_upgrade *find_tower_upgrade(
    const game_config &config,
    tower_type type,
    int level
)
{
    for (int i = 0; i < length(config.tower_upgrades); i++)
    {
        if (config.tower_upgrades[i].type == type &&
            config.tower_upgrades[i].level == level)
        {
            return &config.tower_upgrades[i];
        }
    }

    return nullptr;
}