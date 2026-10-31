# Elemental Tower Defence

A tower defence game developed in C++ using SplashKit for
FIT1045 Introduction to Programming.

Players place and upgrade elemental towers to defend against
waves of enemies following a fixed path.

## Requirements

- C++ compiler (clang++)
- SplashKit installed and configured
- JSON configuration files inside Resources/json/

## Build and Run

Open a terminal in the project folder and compile:

```bash
clang++ config.cpp effects.cpp enemy.cpp game.cpp main.cpp map.cpp projectile.cpp render.cpp tower.cpp wave.cpp -o main -l splashkit -Wall -Wextra
```

Run the game on Windows using MSYS2 MinGW64:

```bash
./main.exe
```

Keep the Resources folder in the project directory so
SplashKit can load the JSON configuration files.

## How to Play

1. Click a blue tower spot to open the purchase menu.
2. Choose FIRE, WATER, ELECTRIC or EARTH to purchase a tower.
3. Click an existing tower to upgrade, sell or change its targeting.
4. During preparation, view the upcoming enemies and wait for
   the countdown or click START EARLY.
5. Defeat enemies to earn coins and prevent them from reaching
   the end of the path.

The game ends when the player's lives reach zero.

### Tower Targeting

- FIRST: Target the enemy furthest along the path.
- CLOSEST: Target the enemy nearest to the tower.
- STRONGEST: Target the enemy with the highest current health.
- WEAKEST: Target the enemy with the lowest current health.

Towers can be upgraded to level 5 and sold for 70% of
their total purchase and upgrade costs.

## Enemy Types

The game contains five enemy types:

- NORMAL: Standard enemy.
- FAST: Moves faster than normal enemies.
- TANK: High health and resistance to EARTH projectiles.
- SWARM: Low health and faster spawning.
- BOSS: High health and resistance to direct projectile damage.

Boss enemies appear at the end of every fifth wave.
Enemy health increases as waves progress.

## Elemental Towers and Reactions

Each elemental tower applies a different status effect:

- FIRE: BURNING, which deals damage over time.
- WATER: WET, which slows enemy movement.
- ELECTRIC: CHARGED.
- EARTH: CRACKED.

Combining two elemental statuses can trigger six reactions:

| Reaction | Combination | Result |
|---|---|---|
| ELECTROSHOCK | WET + CHARGED | Damage and stun |
| VAPORIZE | WET + BURNING | Bonus damage |
| MUD TRAP | WET + CRACKED | Damage and immobilisation |
| OVERLOAD | BURNING + CHARGED | Area damage |
| MAGMA BURST | BURNING + CRACKED | Area damage |
| SHATTER | CHARGED + CRACKED | Bonus damage |

Enemies display a status letter inside their circles
to indicate an active elemental effect or restriction.

## JSON Configuration

The game uses six JSON files stored in Resources/json/.

| File | Purpose |
|---|---|
| towers.json | Tower statistics and costs |
| upgrades.json | Tower upgrade statistics and costs |
| enemies.json | Enemy health, speed and rewards |
| map.json | Enemy path and tower positions |
| reactions.json | Elemental reaction statistics |
| wave.json | Wave timing, spawning and rewards |

These files allow gameplay values to be modified without
changing the main C++ source code.

Restart the program after changing a JSON file.

## Code Organisation

- main.cpp: Main game loop and input processing.
- models.h: Structs and enums used by the game.
- game.cpp: Initial game state.
- config.cpp: Loading tower, upgrade and enemy configurations.
- map.cpp: Loading the map and tower positions.
- enemy.cpp: Enemy spawning and movement.
- tower.cpp: Tower placement, targeting, upgrades and selling.
- projectile.cpp: Projectile movement and enemy damage.
- effects.cpp: Elemental statuses and reactions.
- wave.cpp: Wave preparation, spawning and completion.
- render.cpp: Drawing the game, enemies and menus.

## Programming Concepts

The project demonstrates structured procedural programming
using C++ and SplashKit.

The main concepts include:

- Structs and enums for modelling game entities.
- Dynamic arrays for enemies, towers and projectiles.
- Fixed arrays for elemental effects and reaction statistics.
- Functions and procedures for separating responsibilities.
- Loops and conditional statements.
- Searching and comparison algorithms for tower targeting.
- Pass-by-reference for updating game data.
- JSON files for external configuration.
- Input validation and iterative development.

The game does not use object-oriented programming in its model.