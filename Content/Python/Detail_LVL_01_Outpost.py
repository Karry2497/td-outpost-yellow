"""Add a small, repeatable visual-detail pass to the existing Level 1 outpost.

Run in Unreal Editor's Python console:
    py "<ProjectDir>/Content/Python/Detail_LVL_01_Outpost.py"
"""

import unreal


LEVEL_PATH = "/Game/Maps/LVL_01_MorrowDistrict_Walled"
SCALE = 0.8
PREFIX = "Outpost Finish v1/"


def add_box(mesh, material, label, location, dimensions):
    actor = unreal.EditorLevelLibrary.spawn_actor_from_class(
        unreal.StaticMeshActor,
        unreal.Vector(*(value * SCALE for value in location)),
        unreal.Rotator(0.0, 0.0, 0.0),
    )
    actor.set_actor_label(PREFIX + label)
    component = actor.static_mesh_component
    component.set_static_mesh(mesh)
    component.set_world_scale3d(
        unreal.Vector(*(value * SCALE / 100.0 for value in dimensions))
    )
    component.set_material(0, material)
    component.set_collision_enabled(unreal.CollisionEnabled.NO_COLLISION)
    return actor


def main():
    level_editor = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
    if not level_editor.load_level(LEVEL_PATH):
        raise RuntimeError("Could not load Level 1: " + LEVEL_PATH)

    actor_editor = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
    if any(actor.get_actor_label().startswith(PREFIX) for actor in actor_editor.get_all_level_actors()):
        unreal.log("Outpost finish v1 is already present; no duplicates were added")
        return

    cube = unreal.load_asset("/Engine/BasicShapes/Cube.Cube")
    plate = unreal.load_asset("/Game/Maps/Materials/M_Stylized_Interior")
    amber = unreal.load_asset("/Game/Maps/Materials/M_Stylized_BuildPadMark")
    accent = unreal.load_asset("/Game/Maps/Materials/M_Stylized_RoadMark")
    if not all((cube, plate, amber, accent)):
        raise RuntimeError("Required cube or outpost materials are missing")

    for index, x in enumerate((-1050.0, -420.0, 420.0, 1050.0)):
        add_box(cube, plate, "south armor plate %02d" % index,
                (x, -1884.0, 360.0), (430.0, 28.0, 330.0))
        add_box(cube, amber, "south hazard mark %02d" % index,
                (x, -1866.0, 360.0), (230.0, 8.0, 28.0))

    for side, x in (("west", -1884.0), ("east", 1884.0)):
        for index, y in enumerate((-1050.0, -420.0, 420.0, 1050.0)):
            add_box(cube, plate, "%s armor plate %02d" % (side, index),
                    (x, y, 360.0), (28.0, 430.0, 330.0))
            accent_x = x + (4.0 if side == "west" else -4.0)
            add_box(cube, amber, "%s hazard mark %02d" % (side, index),
                    (accent_x, y, 360.0), (8.0, 230.0, 28.0))

    add_box(cube, plate, "command crest plate", (0.0, -1882.0, 480.0), (620.0, 24.0, 220.0))
    add_box(cube, amber, "command crest center", (0.0, -1866.0, 480.0), (230.0, 8.0, 120.0))
    for x in (-205.0, 205.0):
        add_box(cube, accent, "command crest bars", (x, -1866.0, 480.0), (24.0, 8.0, 120.0))

    level_editor.save_current_level()
    unreal.log("Added 28 flush, non-colliding armor and command details to the outpost")


main()
