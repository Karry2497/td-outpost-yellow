# Extraction FPS

Unreal Engine 5.8 C++ third-person tower-defense prototype. Defend the outpost from waves of yellow zombies, improve defenses between attacks, and keep the colorful, graphic presentation readable and playful.

Source repository: [Karry2497/extraction-fps-2](https://github.com/Karry2497/extraction-fps-2) (private).

## Open the project

Open [`extractionFPS.uproject`](extractionFPS.uproject) with Unreal Engine 5.8. The project uses the C++ First Person template and Epic's experimental Unreal MCP plugins (`ModelContextProtocol` and `AllToolsets`).

The Unreal MCP server runs inside the editor on `http://127.0.0.1:8000/mcp`. Keep the editor open while using MCP. It is configured in Codex as `unreal-mcp`. The Python Editor Script Plugin is enabled for the level-generation tools under `Content/Python`.

## First playable loop

The first gameplay slice uses the Morrow District level as a three-lane outpost defense. The native game mode starts a wave after a short setup window; bright yellow walkers, faster runners and armored brutes attack the walls. Kills award credits. Number keys deploy defenses and buy support:

| Key | Action | Cost |
|---|---|---:|
| 1 | Place a sentry turret | 60 |
| 2 | Place a guard bot | 90 |
| 3 | Place barbed wire | 35 |
| 4 | Repair outpost integrity | 40 |
| 5 | Call an eagle airstrike ahead of the player | 120 |
| 6 | Refill current weapon magazine | 25 |
| 7 | Reinforce the outpost walls (up to three times) | 80 |

Purchases are placed just ahead of the player. This is a simple first pass to make the loop playable; placement controls, balance, HUD and final art remain follow-up work.

## First level

The game and editor default map is [`LVL_01_MorrowDistrict_Walled`](Content/Maps/LVL_01_MorrowDistrict_Walled.umap), a city-district greybox with an outpost occupying and blocking an old three-arm junction. Three abandoned road approaches end at the outpost; attached three-storey buildings seal its rear and sides. Three narrow stairs with small individual treads lead from the open, roofless courtyard to connected firing positions. Static wrecked cars remain on the streets. Its layout and regeneration script are documented in [`docs/levels/LEVEL_01_MORROW_DISTRICT.md`](docs/levels/LEVEL_01_MORROW_DISTRICT.md). The optional [`Apply_Morrow_Stylized_Palette.py`](Content/Python/Apply_Morrow_Stylized_Palette.py) recolors the existing blockout without rebuilding or replacing level actors.

To transfer the native level and its materials to another UE5 project, use the [Morrow District import package](Exports/LVL_01_MorrowDistrict_UE5_Import.zip). Extract it at the target project root so its `Content` directory merges into the target `Content` directory, then open `/Game/Maps/LVL_01_MorrowDistrict` in Unreal Editor. The attached project already contains this level and opens on it by default.

## Repository setup

- Unreal binary assets use Git LFS. Git LFS must be installed before cloning or pushing asset changes.
- `Binaries`, `DerivedDataCache`, `Intermediate`, `Saved`, and local IDE data are ignored.
- Commits automatically push to the current branch when a remote is reachable. The local Git hook handles the push after each commit; use an explicit commit when an iteration is ready to share.

## AI iteration loop

Each gameplay or tooling iteration records its goal, acceptance criteria, implementation, in-editor verification, and learning in [`docs/AI/ITERATION_LOOP.md`](docs/AI/ITERATION_LOOP.md). Durable project decisions live in [`docs/AI/PROJECT_MEMORY.md`](docs/AI/PROJECT_MEMORY.md), and reusable lessons are summarized in [`docs/AI/LEARNING_LOG.md`](docs/AI/LEARNING_LOG.md).
