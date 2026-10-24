#include "game.h"

game_data create_game()
{
    game_data game{};

    game.coins = 130;
    game.lives = 10;
    game.wave = 1;

    return game;
}