# Project memory

## Confirmed setup

- Project: casual, single-player, third-person 3D tower-defense game set at the fortified Morrow District outpost. A first playable prototype is the immediate target.
- Engine: Unreal Engine 5.8.
- Foundation: C++ First Person template with Blueprint iteration support. The player camera and movement have been converted to third-person follow-camera controls while keeping the existing weapon-camera accessor intact.
- Unreal module and project stem: `extractionFPS`.
- Unreal MCP: Epic's built-in experimental `ModelContextProtocol` and `AllToolsets` plugins are enabled in the project file. Codex is configured for the local endpoint `http://127.0.0.1:8000/mcp`.
- GitHub: private repository `Karry2497/extraction-fps-2`.
- Source control: Git with Git LFS for Unreal packages and selected large binary assets.
- Iteration records: see `ITERATION_LOOP.md`; reusable learning belongs in `LEARNING_LOG.md`.

## Decisions still open

- Final game title and story framing.
- Exact wave pacing, enemy roster, and mission length.
- Which upgrades are part of the first playable build versus later iterations.
- Target platforms and minimum hardware.

## Confirmed game direction

- Defend the existing roofless outpost in the three-road Morrow District against incoming zombie waves.
- Zombies must be unmistakably yellow in every gameplay view.
- Use the linked OSPUZE advertisement as broad inspiration for bold color, high contrast, energetic composition, and graphic readability. Create an original game identity and assets rather than copying its branding or exact artwork.
- The player controls a visible character in 3D from an elevated over-the-shoulder view.
- The planned defense shop includes bots, turrets, barbed wire, stronger walls, ammunition, and an eagle airstrike. Prices and balance remain to be tuned after a playable loop exists.
- First C++ gameplay pass now has three zombie archetypes (walker, runner, brute), wave pacing, credits, an outpost integrity pool, and hotkeys 1–7 for sentry, guard bot, wire, repairs, airstrike, ammo, and wall reinforcement. The current deployables are colored blockout meshes; their placement, UI, effects, and balance need a playtest pass.
- Morrow District's in-editor palette script has been applied to its existing blockout materials in a dark navy, cyan, teal, and orange scheme. It preserves the map actors and leaves the required yellow zombie color unobstructed. The free Hotstrike Studio stylized zombie has been imported as a skeletal mesh, with a new yellow material that keeps its albedo detail. The asset pack has no animation clips, so the mesh still needs a retargeted idle/walk/attack set before it is ready for live gameplay.
- The latest camera pass uses a shorter 650 cm spring arm and a 48-degree elevated angle. The source compiles, but the updated game could not be runtime-checked because Windows Code Integrity blocked the newly built project DLL under the active enterprise signing policy (events 3077/3033). Do not change or bypass that policy; use an administrator-approved signing or development workflow.
- The active editor can block native C++ rebuilds when Live Coding is enabled. Save work and close the editor or use its Live Coding compile shortcut before rebuilding; do not terminate the editor process because it may contain unsaved map changes.
