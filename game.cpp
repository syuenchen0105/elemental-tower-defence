#include "models.h"

game_data create_game()
{
    game_data game;

    game.coins = 500;
    game.lives = 5;
    game.wave = 0;

    return game;
}