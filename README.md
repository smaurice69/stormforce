# Fieldrunners: Frontiers Prototype

Fieldrunners: Frontiers is an experimental Windows prototype built in C++20 for Visual Studio 2022. It mixes free-pathing tower defense battles with a lightweight world map campaign. The code base is intentionally modular so the core engine (windowing, rendering, input, data management) is separated from the game layer.

## Building

1. Open `FieldrunnersFrontiers.sln` in Visual Studio 2022.
2. Ensure the `x64` platform is active (Debug or Release).
3. Build the `Game` project. The solution outputs to `build/` inside the repository.

The project has no external binary dependencies. All data lives in the `Data/` directory and is loaded at runtime.

## Running

Launch the `FieldrunnersFrontiers.exe` produced in the build output. The executable expects the working directory to contain the `Data/` folder (for towers, enemies, waves, and world maps). When running from Visual Studio this is configured automatically.

## Controls & Flow

### World Map
- **Arrow Keys** – Cycle through frontline sectors.
- **Enter** – Deploy to the selected sector.
- **F5** – Save campaign and in-progress battles.
- **F9** – Load the most recent save.

### Sector Battles
- **Left Click** – Place the selected tower on open ground.
- **Right Click** – Sell the tower under the cursor (refunds half the cost).
- **Number Keys (1-9)** – Swap between tower blueprints.
- **Space** – Toggle fast-forward.
- **Enter** – Advance past victory/defeat screens.

### Loop Overview
1. Choose a frontline sector on the campaign map.
2. Defend the lane in a free-form battle – build towers to reshape the enemy path.
3. Win to push the front forward (gain resources) or lose and see the enemy reinforce.
4. Repeat across 15 connected sectors. Progress, resources, and active battles can be saved/loaded at any time.

## Data-Driven Content

Content lives in JSON files under `Data/`:
- `towers.json` – Tower statistics and cosmetics.
- `enemies.json` – Enemy attributes and bounties.
- `waves.json` – Wave sets referenced by sectors.
- `world.json` – Campaign graph describing sector layout and difficulties.

The engine monitors these files in Debug builds and hot-reloads when they change.

## Saving & Loading

Save data is written to `saves/save.json` beside the executable. The save captures world ownership, resources, day count, and an in-battle snapshot (credits, lives, elapsed time, placed towers). Loading while a battle is active will resume the fight exactly where it left off.

## Debugging Tips

- `BattleState` and `WorldState` keep the simulation deterministic and free of heap churn. The renderer uses a simple GDI blitter for crisp orthographic presentation.
- Toggle `ENGINE_BUILD` / `GAME_BUILD` preprocessor flags to inject diagnostics as needed.

Enjoy experimenting with the front line!
