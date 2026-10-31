#include "wave.h"
#include "enemy.h"

// Load and validate the wave settings from wave.json
bool load_wave_config(wave_config &settings)
{
    json wave_data = json_from_file("wave.json");

    // Check that all required settings are present
    if (
        !json_has_key(wave_data, "first_wave_enemies") ||
        !json_has_key(wave_data, "enemies_added_per_wave") ||
        !json_has_key(wave_data, "regular_spawn_interval_ms") ||
        !json_has_key(wave_data, "swarm_spawn_interval_ms") ||
        !json_has_key(wave_data, "preparation_seconds") ||
        !json_has_key(wave_data, "completion_reward"))
    {
        write_line("Error: wave.json is missing required settings.");

        free_json(wave_data);
        return false;
    }

    // Read each setting from JSON
    int first_wave_enemies = json_read_number_as_int(
        wave_data,
        "first_wave_enemies");

    int enemies_added_per_wave = json_read_number_as_int(
        wave_data,
        "enemies_added_per_wave");

    int regular_interval = json_read_number_as_int(
        wave_data,
        "regular_spawn_interval_ms");

    int swarm_interval = json_read_number_as_int(
        wave_data,
        "swarm_spawn_interval_ms");

    double preparation_seconds = json_read_number_as_double(
        wave_data,
        "preparation_seconds");

    int completion_reward = json_read_number_as_int(
        wave_data,
        "completion_reward");

    // Reject settings that would create invalid or unusable waves
    bool valid =
        first_wave_enemies > 0 &&
        enemies_added_per_wave >= 0 &&
        regular_interval > 0 &&
        swarm_interval > 0 &&
        preparation_seconds >= 0.0 &&
        completion_reward >= 0;

    if (!valid)
    {
        write_line("Error: Invalid values in wave.json.");

        free_json(wave_data);
        return false;
    }

    // Store the validated settings in the game configuration
    wave_config loaded{};

    loaded.first_wave_enemies = first_wave_enemies;
    loaded.enemies_added_per_wave = enemies_added_per_wave;

    // Store the validated enemy spawn intervals
    loaded.regular_spawn_interval_ms = regular_interval;
    loaded.swarm_spawn_interval_ms = swarm_interval;

    loaded.preparation_seconds = preparation_seconds;
    loaded.completion_reward = completion_reward;

    settings = loaded;

    // Release the JSON resource after loading
    free_json(wave_data);

    return true;
}

// Calculate the number of enemies using the configured wave scaling
int get_enemies_per_wave(const game_data &game)
{
    return game.config.waves.first_wave_enemies +
           (game.wave - 1) *
               game.config.waves.enemies_added_per_wave;
}

// Determine the spawn interval for a specific enemy type
static unsigned int get_spawn_interval(
    enemy_type type,
    const wave_config &settings)
{
    // SWARM enemies use their own faster spawn interval
    if (type == SWARM)
    {
        return settings.swarm_spawn_interval_ms;
    }

    return settings.regular_spawn_interval_ms;
}

// Update preparation timing and spawn enemies during active waves
void update_wave(
    game_data &game,
    wave_state &state,
    unsigned int current_time)
{
    double current_time_seconds = current_time / 1000.0;

    // Begin the wave when the preparation countdown finishes
    if (game.phase == PREPARATION)
    {
        if (current_time_seconds < game.preparation_end_time)
        {
            return;
        }

        game.phase = IN_WAVE;
    }

    if (game.phase != IN_WAVE)
    {
        return;
    }

    // Initialise spawning when a new wave begins
    if (!state.started)
    {
        state.last_spawn_time = current_time;
        state.started = true;
    }

    int enemies_per_wave = get_enemies_per_wave(game);

    // Stop spawning once the required number has been reached
    if (state.spawned_count >= enemies_per_wave)
    {
        return;
    }

    // Determine the next enemy type using the existing wave rules
    enemy_type next_type = get_wave_enemy_type(
        game.wave,
        state.spawned_count,
        enemies_per_wave);

    unsigned int spawn_interval = get_spawn_interval(
        next_type,
        game.config.waves);

    // Spawn the next enemy after its configured interval
    if (current_time - state.last_spawn_time >= spawn_interval)
    {
        spawn_enemy(game, next_type);

        state.spawned_count++;
        state.last_spawn_time = current_time;
    }
}

// Complete the wave when all enemies have spawned and left the game
void complete_wave_if_finished(
    game_data &game,
    wave_state &state,
    double current_time)
{
    // Do not complete waves when the player has lost
    if (game.lives <= 0)
    {
        return;
    }

    if (game.phase != IN_WAVE)
    {
        return;
    }

    int enemies_per_wave = get_enemies_per_wave(game);

    // Wait until all scheduled enemies have spawned
    if (state.spawned_count < enemies_per_wave)
    {
        return;
    }

    // Wait until all enemies are defeated or have escaped
    if (length(game.enemies) > 0)
    {
        return;
    }

    // Award the completion reward configured in wave.json
    game.coins += game.config.waves.completion_reward;

    // Advance to the next wave
    game.wave++;

    // Reset the spawning state for the upcoming wave
    state.spawned_count = 0;
    state.last_spawn_time = 0;
    state.started = false;

    // Start the configured preparation countdown
    game.phase = PREPARATION;
    game.preparation_end_time =
        current_time + game.config.waves.preparation_seconds;
}