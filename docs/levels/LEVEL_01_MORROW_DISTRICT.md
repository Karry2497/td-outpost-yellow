# Level 01: Morrow District

## Purpose

First playable greybox for the extraction-focused FPS. The old three-arm street junction has been occupied by a fortified zombie-control outpost. Its courtyard is the safe spawn and control point; the outpost blocks the old road, and attached city buildings close the rear and sides so there is no route around it. Only static wrecks remain on the three approach streets. It uses Unreal's built-in primitive meshes, so it needs no external art pack.

## Player route

1. Spawn inside the outpost courtyard.
2. Use one of the three stairs to climb to a road-facing firing wall.
3. Move along the connected wall-top walk between firing positions and watch the three blocked approaches.
4. Use the clear courtyard as the staging point; zombie spawning, counting and combat rules are subsequent gameplay work.

## Greybox footprint

- Playable footprint: 240 m east-west by 200 m north-south.
- Primary lanes: three abandoned road approaches from the west, east and south. Each ends at an outpost wall; none passes through or around the courtyard.
- Buildings: three-storey windowed blocks border the outer edge of the firing walk along the rear and sides. Their corners leave the full west and east walls and the connected walk exposed; sealed corner blocks close ground-level bypass gaps. A connected rear support building closes the northern edge without blocking the walk.
- Wreckage: damaged, stationary cars and rubble sit on the approach roads. No traffic or moving vehicles are present.
- The outpost occupies the old T-junction. Its courtyard has no roof, center building, console or towers.
- The outpost has a continuous 6.2 m wall and connected wall-top walk, visually distinguished from the surrounding buildings. Three narrow stairs, each with 40 small treads, lead to the west, east and south shooting positions. Their offset entrances keep the courtyard center clear. The rear walk links the three positions, while merlons leave firing gaps on the road-facing walls.

## Acceptance criteria for this slice

- `LVL_01_MorrowDistrict_Walled` exists under `/Game/Maps` and opens as the project default level.
- `LVL_01_MorrowDistrict_Walled` opens with PlayerStart inside the outpost courtyard.
- Three visible road stubs reach the outpost from the west, east and south, then stop at its walls.
- No road or traversable gap runs through or around the outpost; building masses and sealed corner blocks close its rear and sides without covering the firing walls or walk.
- Each of the three road-facing shooting walls has one stair from the courtyard to the connected wall-top walk.
- The courtyard center stays clear, and the stairs use narrow individual treads rather than oversized steps.
- Static wrecked cars make clear that traffic has stopped after the apocalypse.
- District buildings have three visible storeys and dark blockout window panes.
- No roof or tower covers the courtyard.
- No imported art, gameplay system, or online dependency is required for the blockout.

## Build

Open the project in Unreal Editor with the Python Editor Script Plugin enabled, then run `Content/Python/Build_LVL_01_MorrowDistrict.py` in the Python console. Re-running the script regenerates the walled level and its blockout materials.

This is a greybox slice. Zombie spawning, counting, combat logic, extraction interactions, final art, lighting and playtest tuning remain subsequent iterations.
