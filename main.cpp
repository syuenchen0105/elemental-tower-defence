#include "splashkit.h"
#include "config.h"
#include "effects.h"
#include "enemy.h"
#include "game.h"
#include "map.h"
#include "projectile.h"
#include "render.h"
#include "tower.h"
#include "wave.h"

// Initialise the game and run the main gameplay loop
int main()
{
    game_data game = create_game();

    // Load tower, upgrade and enemy configurations
    load_tower_stats(game.config.tower_definitions);
    load_tower_upgrades(game.config.tower_upgrades);
    load_enemy_stats(game.config.enemy_definitions);

    // Load the enemy path and tower spots from map.json
    if (!load_map(game))
    {
        write_line("Unable to start the game: map loading failed.");
        return 1;
    }

    open_window("Tower Defence", 1000, 700);

    // Create a timer for spawning and gameplay effects
    timer spawn_timer = create_timer("Spawn Timer");
    start_timer(spawn_timer);

    // Store wave spawning progress separately from other game data
    wave_state state{};

    bool menu_was_open = false;

    do
    {
        process_events();

        // Handle clicks on empty and occupied tower spots
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

        // Read the current gameplay time
        unsigned int current_time = timer_ticks(spawn_timer);
        double current_time_seconds = current_time / 1000.0;

        game.current_time = current_time_seconds;

        // Update gameplay while the player is alive and menus are closed
        if (game.lives > 0 && !menu_is_open)
        {
            // Update wave preparation and enemy spawning
            update_wave(
                game,
                state,
                current_time);

            if (game.phase == IN_WAVE)
            {
                // Move active enemies along the path
                update_enemies(
                    game.enemies,
                    game.path,
                    game.lives,
                    current_time_seconds);

                // Allow towers to find targets and fire projectiles
                update_tower_attacks(
                    game,
                    current_time_seconds);

                // Move projectiles and process elemental reactions
                update_projectiles(
                    game.projectiles,
                    game.enemies,
                    game.coins,
                    current_time_seconds);

                // Apply ongoing effects such as BURNING damage
                update_enemy_effects(
                    game.enemies,
                    current_time_seconds,
                    game.coins);

                // Remove defeated enemies and inactive projectiles
                remove_inactive_enemies(game.enemies);
                remove_inactive_projectiles(game.projectiles);

                // Check whether the wave has finished
                complete_wave_if_finished(
                    game,
                    state,
                    current_time_seconds);
            }
        }

        // Draw the map, towers, enemies and interface
        draw_game(game);
        draw_interface();
        refresh_screen(60);

    } while (!quit_requested());

    return 0;
}