#ifndef MODELS_H
#define MODELS_H

#include "splashkit.h"
#include "splashkit-arrays.h"

// Different enemy types with unique statistics
enum enemy_type
{
    NORMAL,
    FAST,
    TANK,
    SWARM,
    BOSS
};

// Four elemental tower types
enum tower_type
{
    FIRE,
    WATER,
    ELECTRIC,
    EARTH
};

// Strategies used by towers to select enemies
enum targeting_mode
{
    FIRST,
    CLOSEST,
    STRONGEST,
    WEAKEST
};

// Elemental status effects applied to enemies
enum effect_type
{
    BURNING,
    WET,
    CHARGED,
    CRACKED,
    EFFECT_COUNT
};

// Six possible elemental reactions
enum reaction_type
{
    ELECTROSHOCK,
    VAPORIZE,
    MUD_TRAP,
    OVERLOAD,
    MAGMA_BURST,
    SHATTER,
    REACTION_COUNT
};

// Different phases of the wave system
enum game_phase
{
    PREPARATION,
    IN_WAVE
};

// Base tower statistics loaded from JSON
struct tower_stats
{
    tower_type type;
    int cost;
    int damage;
    double range;
    double attack_delay;
};

// Tower upgrade statistics loaded from JSON
struct tower_upgrade
{
    tower_type type;
    int level;
    int cost;
    int damage_increase;
    double range_increase;
    double attack_delay_reduction;
};

// Base enemy statistics loaded from JSON
struct enemy_stats
{
    enemy_type type;
    int health;
    double speed;
    int reward;
};

// Statistics for an individual elemental reaction
struct reaction_stats
{
    int damage = 0;
    double radius = 0.0;
    double control_duration = 0.0;
    double cooldown = 0.0;
};

// Shared settings for all elemental reactions
struct reaction_config
{
    fixed_array<reaction_stats, REACTION_COUNT> definitions{};

    int max_aoe_targets = 3;
    double stun_cooldown = 3.0;
    double boss_control_multiplier = 0.5;
};

// Configuration for wave timing, spawning and rewards
struct wave_config
{
    int first_wave_enemies = 3;
    int enemies_added_per_wave = 1;

    unsigned int regular_spawn_interval_ms = 1000;
    unsigned int swarm_spawn_interval_ms = 350;

    double preparation_seconds = 10.0;
    int completion_reward = 25;
};

// Stores all configuration data used by the game
struct game_config
{
    dynamic_array<tower_stats> tower_definitions;
    dynamic_array<tower_upgrade> tower_upgrades;
    dynamic_array<enemy_stats> enemy_definitions;

    reaction_config reactions;
    wave_config waves;
};

// Tracks elemental status durations and movement restrictions
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

// Represents an enemy moving along the path
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

// Represents a tower purchased and upgraded by the player
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

// Represents a projectile travelling toward an enemy
struct projectile
{
    point_2d position;
    int target_enemy_id;
    tower_type type;

    int damage;
    double speed;
    bool active;
};

// Represents a fixed location where a tower can be placed
struct tower_spot
{
    int id;
    point_2d position;
    bool occupied;
    int tower_id;
};

// Stores the current game state and all active entities
struct game_data
{
    game_config config;

    dynamic_array<enemy> enemies;
    dynamic_array<tower> towers;
    dynamic_array<projectile> projectiles;
    dynamic_array<tower_spot> tower_spots;
    dynamic_array<point_2d> path;

    // Player resources and current wave
    int coins;
    int lives;
    int wave;

    // Unique identifiers for new entities
    int next_enemy_id = 1;
    int next_tower_id = 1;
    int next_spot_id = 1;

    // Tower menu selection and visibility
    bool purchase_menu_open = false;
    bool management_menu_open = false;
    int selected_spot_index = -1;

    // Wave preparation and gameplay timing
    game_phase phase = PREPARATION;
    double current_time = 0.0;
    double preparation_end_time = 10.0;
};

#endif