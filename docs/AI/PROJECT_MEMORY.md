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
