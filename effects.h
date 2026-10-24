#ifndef EFFECTS_H
#define EFFECTS_H

#include "models.h"

void apply_effect(
    enemy &current_enemy,
    effect_type type,
    double current_time,
    double duration
);

bool is_effect_active(
    const enemy &current_enemy,
    effect_type type,
    double current_time
);

#endif