#include "effects.h"

static void extend_effect(
    double &end_time,
    double current_time,
    double duration
)
{
    double new_end_time = current_time + duration;

    if (new_end_time > end_time)
    {
        end_time = new_end_time;
    }
}

void apply_effect(
    enemy &current_enemy,
    effect_type type,
    double current_time,
    double duration
)
{
    extend_effect(
        current_enemy.effects.end_times[type],
        current_time,
        duration
    );
}

bool is_effect_active(
    const enemy &current_enemy,
    effect_type type,
    double current_time
)
{
    return current_time < current_enemy.effects.end_times[type];
}