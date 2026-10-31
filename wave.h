#ifndef WAVE_H
#define WAVE_H

#include "models.h"

// Tracks enemy spawning progress during the current wave
struct wave_state
{
    // Number of enemies already spawned
    int spawned_count = 0;

    // Time when the previous enemy was spawned, in milliseconds
    unsigned int last_spawn_time = 0;

    // Indicates whether the current wave has started spawning
    bool started = false;
};

// Update wave preparation and spawn enemies at the required intervals
void update_wave(
    game_data &game,
    wave_state &state,
    unsigned int current_time);

// Complete the current wave, award coins and start the next preparation phase
void complete_wave_if_finished(
    game_data &game,
    wave_state &state,
    double current_time);

#endif