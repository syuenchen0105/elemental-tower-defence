#include "wave.h"
#include "enemy.h"

// Calculate the total number of enemies in a wave
static int get_enemies_per_wave(int wave)
{
    return wave + 2;
}

// Determine how long to wait before spawning the next enemy
static unsigned int get_spawn_interval(enemy_type type)
{
    // SWARM enemies spawn more frequently than other enemy types
    if (type == SWARM)
    {
        return 350;
    }

    return 1000;
}

// Handle the preparation countdown and spawn enemies during a wave
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

    // Initialise the spawn timer when a new wave begins
    if (!state.started)
    {
        state.last_spawn_time = current_time;
        state.started = true;
    }

    int enemies_per_wave = get_enemies_per_wave(game.wave);

    // Stop spawning once all enemies for this wave have appeared
    if (state.spawned_count >= enemies_per_wave)
    {
        return;
    }

    // Determine the next enemy type based on the wave number
    enemy_type next_type = get_wave_enemy_type(
        game.wave,
        state.spawned_count,
        enemies_per_wave);

    unsigned int spawn_interval = get_spawn_interval(next_type);

    // Spawn the next enemy when its interval has elapsed
    if (current_time - state.last_spawn_time >= spawn_interval)
    {
        spawn_enemy(game, next_type);

        state.spawned_count++;
        state.last_spawn_time = current_time;
    }
}

// Finish the wave when all enemies have spawned and none remain
void complete_wave_if_finished(
    game_data &game,
    wave_state &state,
    double current_time)
{
    // Do not complete a wave after the player loses
    if (game.lives <= 0)
    {
        return;
    }

    if (game.phase != IN_WAVE)
    {
        return;
    }

    int enemies_per_wave = get_enemies_per_wave(game.wave);

    // Wait until every enemy has been spawned
    if (state.spawned_count < enemies_per_wave)
    {
        return;
    }

    // Wait until all enemies are defeated or have reached the end
    if (length(game.enemies) > 0)
    {
        return;
    }

    // Reward the player for completing the wave
    game.coins += 25;

    // Advance to the next wave
    game.wave++;

    // Reset the spawning state for the new wave
    state.spawned_count = 0;
    state.last_spawn_time = 0;
    state.started = false;

    // Give the player 10 seconds to prepare
    game.phase = PREPARATION;
    game.preparation_end_time = current_time + 10.0;
}