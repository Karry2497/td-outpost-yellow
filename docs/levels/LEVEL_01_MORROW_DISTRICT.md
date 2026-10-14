# Level 01: Morrow District

## Purpose

First playable greybox for the extraction-focused FPS. This establishes one compact urban route with an insertion point, a visible objective landmark, multiple approaches, combat cover and an extraction zone. It uses Unreal's built-in cube and cylinder meshes, so it needs no external art pack.

## Player route

1. Insert on the south-west end of the district, facing the insertion street.
2. Move east along the insertion street to the north-south avenue.
3. Reach the clinic data cache beside the market junction. The cyan plinth is a visual placeholder; loot interaction is not implemented yet.
4. Reach the green extraction marker in the north-east. The avenue, west service alley and east service alley each connect with the north extraction road, providing alternate approaches and sightlines.

## Greybox footprint

- Playable footprint: 240 m east-west by 200 m north-south.
- Primary lanes: insertion street, market street and the south approach that terminates at a three-arm T-junction.
- Secondary lanes: west and east service alleys, with short sidewalk routes along the main streets.
- Landmarks: three-storey windowed shops, clinic and apartments; a fortified outpost centered on the T-junction; a long three-storey support building behind it; the clinic cache and extraction shelter.
- Combat spaces: open junction, street barriers, crate stacks, hollow building shells, short interior partitions and low roofs.
- The intersection has three arms (west, east and south) and visible traffic signals; there is no northbound road continuing through the junction.
- The outpost is an open courtyard with a continuous 6.2 m solid wall, a connected firing walk with gaps between cover merlons, and three interior stairways to the wall top. It has no center building, roof or corner towers.

## Acceptance criteria for this slice

- `LVL_01_MorrowDistrict` exists under `/Game/Maps` and opens as an Unreal level.
- A PlayerStart is placed at the insertion end.
- The clinic cache and extraction zone are easy to distinguish by material color.
- At least two alternate routes connect the market area with the extraction approach.
- All district buildings have three visible storeys and dark blockout window panes.
- The outpost is centered at the three-arm junction; its wall-top firing walk connects continuously around the corners.
- Three stairways connect the courtyard to the connected wall walk; no towers or roof cover the courtyard.
- No imported art, gameplay system, or online dependency is required for the blockout.

## Build

Open the project in Unreal Editor with the Python Editor Script Plugin enabled, then run `Content/Python/Build_LVL_01_MorrowDistrict.py` in the Python console. Re-running the script replaces the level and its blockout materials.

This is a greybox slice. Extraction interactions, objective logic, AI placement, final art, lighting and playtest tuning remain subsequent iterations.
