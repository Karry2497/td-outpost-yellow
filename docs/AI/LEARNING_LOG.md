# Reusable learning

Add a short entry only when an iteration produces a specific, repeatable lesson that can improve later work. Link the commit or iteration when useful. Do not store credentials, personal data, or speculative conclusions here.

## Lessons

- In Unreal Engine 5.8, run project Python builders with `UnrealEditor-Cmd.exe <project.uproject> -run=pythonscript -script=<script.py>`. Use `LevelEditorSubsystem` for creating, loading and saving maps; `EditorLevelLibrary` emits deprecation warnings.
- A Python builder that regenerates the active map should clear its actors in place. `NewLevel` rejects a map package that already exists, even after an editor asset deletion attempt.
- To move a native UE5 level between projects, preserve the `.umap` and its referenced `.uasset` dependencies under the same `Content` paths, or use Content Browser > Asset Actions > Migrate. The generic Import action is for interchange formats and does not import native `.umap` packages.
- For a defensible greybox outpost, model the wall as continuous collision geometry and make the wall-top firing path continuous at corners; put stairs on the safe interior side and leave firing gaps between cover merlons.
- When an outpost occupies a street junction, stop each approach at the outer wall and attach surrounding building footprints flush to the rear and side walls. Do not add a loop road or traffic path around a fortified position intended to seal the junction; represent the apocalypse with stationary wrecks.
- Keep surrounding building footprints outside the full wall-top walk footprint, including its corners; close the ground-level bypass gaps with corner infill that ends at the walk surface so it does not obstruct the firing route.
- Give the outpost walls and firing walk their own blockout material so the defensive perimeter remains readable against large surrounding building masses in an overhead editor view.
- Size stair runs from the courtyard center to one wall (`compound_half`), not across the entire courtyard; for the 7 m wall walk, 40 treads yield roughly 17.5 cm rise and 37.5 cm depth.
