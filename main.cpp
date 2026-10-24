#include "config.h"
#include "enemy.h"
#include "game.h"
#include "map.h"
#include "models.h"
#include "projectile.h"
#include "render.h"
#include "tower.h"

int main()
{
    // Initialise game data
    game_data game = create_game();

    // Load game configuration
    load_tower_stats(game.config.tower_definitions);
    load_tower_upgrades(game.config.tower_upgrades);

    setup_path(game.path);
    setup_tower_spots(game);

    open_window("Tower Defence", 1000, 700);

    // Initialise spawn timer
    timer spawn_timer = create_timer("Spawn Timer");
    start_timer(spawn_timer);

    int spawned_count = 0;
    unsigned int last_spawn_time = 0;

    bool menu_was_open = false;

    do
    {
        process_events();

        if (game.lives > 0)
        {
            handle_tower_click(game);
        }

        bool menu_is_open = game.purchase_menu_open || game.management_menu_open;

        // Pause or resume the timer when either menu opens or closes
        if (menu_is_open && !menu_was_open)
        {
            pause_timer(spawn_timer);
        }
        else if (!menu_is_open && menu_was_open)
        {
            resume_timer(spawn_timer);
        }

        menu_was_open = menu_is_open;

        if (game.lives > 0 && !menu_is_open)
        {
            int enemies_per_wave = game.wave + 2;
            unsigned int current_time = timer_ticks(spawn_timer);

            // Spawn one enemy every second
            if (spawned_count < enemies_per_wave &&
                current_time - last_spawn_time >= 1000)
            {
                spawn_enemy(game);
                spawned_count++;
                last_spawn_time = current_time;
            }

            // Update enemy movement
            update_enemies(game.enemies, game.path, game.lives);

            // Update tower attacks
            double current_time_seconds = current_time / 1000.0;
            update_tower_attacks(game, current_time_seconds);

            // Update projectile movement and damage
            update_projectiles(
                game.projectiles,
                game.enemies,
                game.coins,
                current_time_seconds
            );

            // Remove inactive enemies and projectiles
            remove_inactive_enemies(game.enemies);
            remove_inactive_projectiles(game.projectiles);

            // Start the next wave when all enemies are gone
            if (game.lives > 0 &&
                spawned_count == enemies_per_wave &&
                length(game.enemies) == 0)
            {
                game.wave++;
                spawned_count = 0;
                last_spawn_time = current_time;
            }
        }

        // Draw the current game state
        draw_game(game);
        draw_interface();
        refresh_screen(60);

    } while (!quit_requested());

    return 0;
}