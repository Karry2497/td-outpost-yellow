# Level 01: Morrow District

## Purpose

First playable greybox for a simple, 3D tower-defense extraction game. There is no controllable survivor character on the map: the player commands the defense through the build interface. The old three-arm street junction has been occupied by a fortified zombie-control outpost. Yellow zombies arrive in readable waves along the three approach streets and attack the outpost if they reach its walls. The player spends earned credits on a small set of defenses, ammunition and repairs; deployed defenses attack automatically. Keep the outpost standing through the final wave to secure extraction at the helicopter zone outside town. The outpost blocks the old road, and attached city buildings close the rear and sides so there is no route around it. Only static wrecks remain on the three approach streets. It uses Unreal's built-in primitive meshes, so it needs no external art pack.

## Player route

1. Start at the command view over the outpost; no character pawn appears in the level.
2. Protect the walls from yellow zombie waves coming down the west, east and south roads.
3. Spend credits through the build interface on a small set of defenses, ammo and repairs; defenses attack automatically.
4. Keep the outpost standing through all five waves. If its integrity reaches zero, the run ends.
5. Clear the final wave and hold the helicopter extraction zone until evacuation completes.

## Tower-defense reference target

Use roughly 75% of the linked tower-defense video as inspiration for the tower-and-upgrade feel, while keeping this game's own yellow-zombie, post-apocalyptic outpost and helicopter-extraction identity. The first pass keeps the choices readable: three deployables, three turret upgrade tiers, and simple on-screen credit feedback. Turret upgrades increase damage, fire rate and range; tier costs are 50, 100 and 150 credits. Six marked shoulder pads (two per approach) suggest useful positions without narrowing the zombie lanes. This is a design target based on the user's stated focus; the linked video could not be loaded directly for detailed shot-by-shot reference.

## Eagle Airstrike

Press 5 to open a paused target-selection menu, then choose the west, east or south approach with a button or keys 1–3. The strike deals 250 damage to every zombie assigned to that lane, regardless of distance, and costs 240 credits. Escape cancels selection without spending credits; selecting an empty lane also does not charge.

## Greybox footprint

- Playable footprint: 240 m east-west by 200 m north-south.
- Primary lanes: three abandoned road approaches from the west, east and south. Each ends at an outpost wall; none passes through or around the courtyard.
- Tower placement: six clearly marked, low build pads sit on the shoulders of the three approach lanes, two per lane. They keep the zombie paths clear and give towers overlapping coverage options.
- Selecting a turret enters placement mode and shows yellow downward arrows above each available rooftop pad. Click an arrow to place and pay; occupied pads are hidden. Right-click or Escape cancels without spending credits.
- Buildings: three-storey windowed blocks border the outer edge of the firing walk along the rear and sides. Their corners leave the full west and east walls and the connected walk exposed; sealed corner blocks close ground-level bypass gaps. A connected rear support building closes the northern edge without blocking the walk.
- Wreckage: damaged, stationary cars and rubble sit on the approach roads. No traffic or moving vehicles are present.
- Extraction: the southern street is visually extended beyond the city edge to an orange-bordered landing apron, with an H touchdown mark and a simple parked helicopter greybox. This communicates the extraction destination; an unlockable player route and extraction trigger remain gameplay tasks.
- The outpost occupies the old T-junction. Its courtyard has no roof, center building, console or towers.
- The outpost has a continuous 6.2 m high, 8 m thick wall base that fills the full wall-top walk footprint down to ground level, leaving no sheltered cavity beneath the walk. Three narrow stairs, each with 40 small treads, reach the inner edge of the wall and lead to the west, east and south shooting positions. Their offset entrances keep the courtyard center clear. The rear walk links the three positions. The three road-facing firing edges stay open, with no high merlons or overhead cover blocking aim.

## Acceptance criteria for this slice

- `LVL_01_MorrowDistrict_Walled` exists under `/Game/Maps` and opens as the project default level.
- `LVL_01_MorrowDistrict_Walled` opens in a raised command camera over the outpost; there is no controllable survivor pawn.
- Three visible road stubs reach the outpost from the west, east and south, then stop at its walls.
- Six non-blocking build pads provide two tower positions on each approach without obstructing the zombie routes.
- No road or traversable gap runs through or around the outpost; building masses and sealed corner blocks close its rear and sides without covering the firing walls or walk. Solid wall geometry fills all space beneath the walk so zombies cannot shelter there.
- Each of the three road-facing shooting walls has one stair from the courtyard to the connected wall-top walk, with open firing edges and unobstructed aim toward each road.
- The courtyard center stays clear, and the stairs use narrow individual treads rather than oversized steps.
- Static wrecked cars make clear that traffic has stopped after the apocalypse.
- District buildings have three visible storeys and dark blockout window panes.
- No roof or tower covers the courtyard or wall-top firing positions.
- A distinct helicopter landing zone is visible outside the city's southern edge and lines up with the extended southern road.
- A raised command camera clearly shows the outpost, three approach lanes, defenses and yellow zombies.
- No imported art, gameplay system, or online dependency is required for the blockout.

## Build

Open the project in Unreal Editor with the Python Editor Script Plugin enabled, then run `Content/Python/Build_LVL_01_MorrowDistrict.py` in the Python console. Re-running the script regenerates the walled level and its blockout materials.

This remains a greybox slice. The native wave, defense and extraction loop is under implementation and needs editor build/PIE verification, final art, lighting and balance tuning. Keep the first release intentionally small: one map, one yellow zombie look with a few simple behavior variants, five readable waves, a raised command camera, automated defenses, and only the core purchases already shown in the HUD. Add more enemy types and upgrades only after this loop is reliable and fun.
