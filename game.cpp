#include "game.h"

// Create a new game with its starting resources
game_data create_game()
{
    // Starting values for a new game
    const int STARTING_COINS = 200;
    const int STARTING_LIVES = 10;
    const int STARTING_WAVE = 1;

    game_data game{};

    game.coins = STARTING_COINS;
    game.lives = STARTING_LIVES;
    game.wave = STARTING_WAVE;

    return game;
}