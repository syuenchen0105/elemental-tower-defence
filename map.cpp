#include "map.h"

void setup_path(dynamic_array<point_2d> &path)
{
    // Define the path from the enemy spawn point to the base
    add(path, point_at(0, 300));
    add(path, point_at(250, 300));
    add(path, point_at(250, 500));
    add(path, point_at(700, 500));
    add(path, point_at(700, 200));
    add(path, point_at(1000, 200));
}

tower_spot create_tower_spot(point_2d position, int id)
{
    tower_spot new_spot = {};

    new_spot.id = id;
    new_spot.position = position;
    new_spot.occupied = false;
    new_spot.tower_id = -1;

    return new_spot;
}

void setup_tower_spots(game_data &game)
{
    dynamic_array<point_2d> positions = {};

    add(positions, point_at(130, 220));
    add(positions, point_at(160, 400));
    add(positions, point_at(350, 400));
    add(positions, point_at(500, 590));
    add(positions, point_at(620, 400));
    add(positions, point_at(790, 400));
    add(positions, point_at(800, 120));
    add(positions, point_at(900, 280));

    // Create tower spots with unique IDs
    for (int i = 0; i < length(positions); i++)
    {
        add(
            game.tower_spots,
            create_tower_spot(positions[i], game.next_spot_id)
        );
        game.next_spot_id++;
    }
}