#ifndef MODELS_H
#define MODELS_H

#include "splashkit.h"
#include "splashkit-arrays.h"

enum enemy_type
{
    NORMAL,
    FAST,
    TANK,
    SWARM,
    BOSS
};

enum tower_type
{
    FIRE,
    WATER,
    ELECTRIC,
    EARTH
};

enum targeting_mode
{
    FIRST,
    CLOSEST,
    STRONGEST,
    WEAKEST
};

enum effect_type
{
    BURNING,
    WET,
    CHARGED,
    CRACKED,
    EFFECT_COUNT
};

struct tower_stats
{
    tower_type type;
    int cost;
    int damage;
    double range;
    double attack_delay;
};

struct tower_upgrade
{
    tower_type type;
    int level;
    int cost;
    int damage_increase;
    double range_increase;
    double attack_delay_reduction;
};

struct enemy_stats
{
    enemy_type type;
    int health;
    double speed;
    int reward;
};

struct game_config
{
    dynamic_array<tower_stats> tower_definitions;
    dynamic_array<tower_upgrade> tower_upgrades;
    dynamic_array<enemy_stats> enemy_definitions;
};

struct enemy_effects
{
    fixed_array<double, EFFECT_COUNT> end_times{0.0};

    double next_burn_tick_time = 0.0;
    double wet_slow_end_time = 0.0;

    double stun_end_time = 0.0;
    double stun_cooldown_end_time = 0.0;
    double immobilize_end_time = 0.0;
    double reaction_cooldown_end_time = 0.0;
};

struct enemy
{
    int id;
    enemy_type type;
    point_2d position;
    int path_index;

    int health;
    int max_health;
    double speed;
    int reward;

    bool active;
    enemy_effects effects;
};

struct tower
{
    int id;
    int spot_id;
    tower_type type;
    targeting_mode targeting;

    int level;
    int cost;
    int total_spent;
    int damage;

    double range;
    double attack_delay;
    double last_attack_time;
};

struct projectile
{
    point_2d position;
    int target_enemy_id;
    tower_type type;

    int damage;
    double speed;
    bool active;
};

struct tower_spot
{
    int id;
    point_2d position;
    bool occupied;
    int tower_id;
};

struct game_data
{
    game_config config;

    dynamic_array<enemy> enemies;
    dynamic_array<tower> towers;
    dynamic_array<projectile> projectiles;
    dynamic_array<tower_spot> tower_spots;
    dynamic_array<point_2d> path;

    int coins;
    int lives;
    int wave;

    int next_enemy_id = 1;
    int next_tower_id = 1;
    int next_spot_id = 1;

    bool purchase_menu_open = false;
    bool management_menu_open = false;
    int selected_spot_index = -1;
};

#endif