#ifndef WAVE_H
#define WAVE_H

#include "models.h"

// Tracks enemy spawning progress during the current wave
struct wave_state
{
    int spawned_count = 0;
    unsigned int last_spawn_time = 0;
    bool started = false;
};

// Load wave timing, enemy count and rewards from wave.json
bool load_wave_config(
    wave_config &settings);

// Calculate how many enemies will appear in the current wave
int get_enemies_per_wave(
    const game_data &game);

// Update wave preparation and spawn enemies at the configured intervals
void update_wave(
    game_data &game,
    wave_state &state,
    unsigned int current_time);

// Complete a wave, award coins and begin the next preparation phase
void complete_wave_if_finished(
    game_data &game,
    wave_state &state,
    double current_time);

#endif