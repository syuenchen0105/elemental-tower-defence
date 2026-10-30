#include "map.h"

void setup_path(
    dynamic_array<point_2d> &path)
{
    add(path, point_2d{0, 300});
    add(path, point_2d{250, 300});
    add(path, point_2d{250, 500});
    add(path, point_2d{700, 500});
    add(path, point_2d{700, 200});
    add(path, point_2d{1000, 200});
}

static void add_tower_spot(
    game_data &game,
    point_2d position)
{
    tower_spot new_spot{};

    new_spot.id = game.next_spot_id++;
    new_spot.position = position;
    new_spot.occupied = false;
    new_spot.tower_id = -1;

    add(game.tower_spots, new_spot);
}

void setup_tower_spots(game_data &game)
{
    add_tower_spot(game, point_2d{130, 220});
    add_tower_spot(game, point_2d{160, 400});
    add_tower_spot(game, point_2d{350, 400});
    add_tower_spot(game, point_2d{500, 590});
    add_tower_spot(game, point_2d{620, 400});
    add_tower_spot(game, point_2d{790, 400});
    add_tower_spot(game, point_2d{800, 120});
    add_tower_spot(game, point_2d{900, 280});
}