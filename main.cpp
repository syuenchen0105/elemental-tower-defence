#include "splashkit.h"
#include "config.h"
#include "effects.h"
#include "enemy.h"
#include "game.h"
#include "map.h"
#include "projectile.h"
#include "render.h"
#include "tower.h"

int main()
{
    game_data game = create_game();

    // Load game configurations
    load_tower_stats(game.config.tower_definitions);
    load_tower_upgrades(game.config.tower_upgrades);
    load_enemy_stats(game.config.enemy_definitions);

    // Set up the map
    setup_path(game.path);
    setup_tower_spots(game);

    open_window("Tower Defence", 1000, 700);

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

        bool menu_is_open = game.purchase_menu_open ||
                            game.management_menu_open;

        // Pause gameplay while a tower menu is open
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
            double current_time_seconds = current_time / 1000.0;

            // Determine the next enemy type
            if (spawned_count < enemies_per_wave)
            {
                enemy_type next_type = get_wave_enemy_type(
                    game.wave,
                    spawned_count,
                    enemies_per_wave);

                unsigned int spawn_interval = 1000;

                // Spawn SWARM enemies more frequently
                if (next_type == SWARM)
                {
                    spawn_interval = 350;
                }

                if (current_time - last_spawn_time >= spawn_interval)
                {
                    spawn_enemy(game, next_type);
                    spawned_count++;
                    last_spawn_time = current_time;
                }
            }

            // Update enemy movement and movement effects
            update_enemies(
                game.enemies,
                game.path,
                game.lives,
                current_time_seconds);

            // Update tower attacks
            update_tower_attacks(
                game,
                current_time_seconds);

            // Update projectiles and elemental reactions
            update_projectiles(
                game.projectiles,
                game.enemies,
                game.coins,
                current_time_seconds);

            // Update BURNING damage over time
            update_enemy_effects(
                game.enemies,
                current_time_seconds,
                game.coins);

            // Remove inactive enemies and projectiles
            remove_inactive_enemies(game.enemies);
            remove_inactive_projectiles(game.projectiles);

            // Advance to the next wave
            if (
                game.lives > 0 &&
                spawned_count == enemies_per_wave &&
                length(game.enemies) == 0)
            {
                game.wave++;
                spawned_count = 0;
                last_spawn_time = current_time;
            }
        }

        // Draw the game and interface
        draw_game(game);
        draw_interface();
        refresh_screen(60);

    } while (!quit_requested());

    return 0;
}