#include "map.h"

// Read a numbered position from a JSON object
static bool read_position(
    json positions,
    int index,
    point_2d &position)
{
    string key = to_string(index);

    // Check whether the requested position exists
    if (!json_has_key(positions, key))
    {
        return false;
    }

    json position_data = json_read_object(positions, key);

    // Each position must contain both x and y coordinates
    if (
        !json_has_key(position_data, "x") ||
        !json_has_key(position_data, "y"))
    {
        free_json(position_data);
        return false;
    }

    // Read the coordinates from JSON
    position.x = json_read_number_as_double(
        position_data,
        "x");

    position.y = json_read_number_as_double(
        position_data,
        "y");

    free_json(position_data);

    return true;
}

// Load the enemy path waypoints in their numbered order
static bool load_path(
    json map_data,
    dynamic_array<point_2d> &path)
{
    if (!json_has_key(map_data, "path"))
    {
        return false;
    }

    json path_data = json_read_object(
        map_data,
        "path");

    int point_count = json_count_keys(path_data);

    // An enemy path must contain at least two waypoints
    if (point_count < 2)
    {
        free_json(path_data);
        return false;
    }

    // Read each waypoint and add it to the path array
    for (int i = 0; i < point_count; i++)
    {
        point_2d position{};

        if (!read_position(path_data, i, position))
        {
            free_json(path_data);
            return false;
        }

        add(path, position);
    }

    free_json(path_data);

    return true;
}

// Create a tower placement spot with a unique ID
static void add_tower_spot(
    game_data &game,
    point_2d position)
{
    tower_spot new_spot{};

    new_spot.id = game.next_spot_id;
    new_spot.position = position;
    new_spot.occupied = false;
    new_spot.tower_id = -1;

    game.next_spot_id++;

    add(game.tower_spots, new_spot);
}

// Load available tower placement spots from JSON
static bool load_tower_spots(
    json map_data,
    game_data &game)
{
    if (!json_has_key(map_data, "tower_spots"))
    {
        return false;
    }

    json spots_data = json_read_object(
        map_data,
        "tower_spots");

    int spot_count = json_count_keys(spots_data);

    // The map must have at least one available tower spot
    if (spot_count < 1)
    {
        free_json(spots_data);
        return false;
    }

    // Create a tower spot for each coordinate stored in JSON
    for (int i = 0; i < spot_count; i++)
    {
        point_2d position{};

        if (!read_position(spots_data, i, position))
        {
            free_json(spots_data);
            return false;
        }

        add_tower_spot(game, position);
    }

    free_json(spots_data);

    return true;
}

// Load and validate the complete map configuration
bool load_map(game_data &game)
{
    // SplashKit searches for this file in Resources/json/
    json map_data = json_from_file("map.json");

    // Check that both required map sections exist
    if (
        !json_has_key(map_data, "path") ||
        !json_has_key(map_data, "tower_spots"))
    {
        write_line("Error: map.json is missing required sections.");

        free_json(map_data);
        return false;
    }

    // Load the path first because enemies depend on its waypoints
    if (!load_path(map_data, game.path))
    {
        write_line("Error: Invalid enemy path in map.json.");

        free_json(map_data);
        return false;
    }

    // Load tower spots after the enemy path
    if (!load_tower_spots(map_data, game))
    {
        write_line("Error: Invalid tower spots in map.json.");

        free_json(map_data);
        return false;
    }

    // Release the JSON resource after reading the configuration
    free_json(map_data);

    return true;
}