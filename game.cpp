#include "game.h"

// Create a new game with its starting resources
game_data create_game()
{
    game_data game{};

    game.coins = 200;
    game.lives = 10;
    game.wave = 1;

    return game;
}