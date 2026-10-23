"""Replace only Level 1 turret pads without rebuilding the whole map."""

import unreal


LEVEL_PATH = "/Game/Maps/LVL_01_MorrowDistrict_Walled"
LEVEL_SCALE = 0.8

PAD_LOCATIONS = (
    ("northwest / inner west", -2300.0, 2300.0),
    ("northwest / outer west", -5000.0, 2300.0),
    ("northwest / rear", -2300.0, 5000.0),
    ("northeast / inner east", 2300.0, 2300.0),
    ("northeast / outer east", 5000.0, 2300.0),
    ("northeast / rear", 2300.0, 5000.0),
    ("southwest / inner west", -2300.0, -2300.0),
    ("southwest / outer west", -5000.0, -2300.0),
    ("southwest / rear", -2300.0, -5000.0),
    ("southeast / inner east", 2300.0, -2300.0),
    ("southeast / outer east", 5000.0, -2300.0),
    ("southeast / rear", 2300.0, -5000.0),
)


def spawn_box(mesh, label, x, y, z, width, depth, height, material):
    actor = unreal.EditorLevelLibrary.spawn_actor_from_class(
        unreal.StaticMeshActor,
        unreal.Vector(x * LEVEL_SCALE, y * LEVEL_SCALE, z * LEVEL_SCALE),
        unreal.Rotator(0.0, 0.0, 0.0),
    )
    if not actor:
        raise RuntimeError("Could not spawn turret pad: " + label)
    actor.set_actor_label(label)
    actor.static_mesh_component.set_static_mesh(mesh)
    actor.set_actor_scale3d(unreal.Vector(width * LEVEL_SCALE / 100.0,
                                         depth * LEVEL_SCALE / 100.0,
                                         height * LEVEL_SCALE / 100.0))
    actor.static_mesh_component.set_material(0, material)
    return actor


def main():
    level_editor = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
    actor_editor = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
    if not level_editor.load_level(LEVEL_PATH):
        raise RuntimeError("Could not load Level 1")

    old_pads = [
        actor for actor in actor_editor.get_all_level_actors()
        if actor.get_actor_label().startswith("Defense build pad /")
    ]
    if old_pads and not actor_editor.destroy_actors(old_pads):
        raise RuntimeError("Could not replace the old turret pads")

    cube = unreal.load_asset("/Engine/BasicShapes/Cube.Cube")
    base_material = unreal.load_asset("/Game/Maps/Materials/M_Stylized_BuildPad")
    mark_material = unreal.load_asset("/Game/Maps/Materials/M_Stylized_BuildPadMark")
    if not cube or not base_material or not mark_material:
        raise RuntimeError("Turret pad mesh or materials could not be loaded")

    for label, x, y in PAD_LOCATIONS:
        prefix = "Defense build pad / " + label
        spawn_box(cube, prefix + " / base", x, y, 1134.0, 300.0, 300.0, 28.0, base_material)
        spawn_box(cube, prefix + " / cross horizontal", x, y, 1150.0, 190.0, 28.0, 4.0, mark_material)
        spawn_box(cube, prefix + " / cross vertical", x, y, 1150.0, 28.0, 190.0, 4.0, mark_material)

    if not level_editor.save_current_level():
        raise RuntimeError("Could not save Level 1")
    unreal.log("Level 1 updated with twelve rooftop turret pads")


main()
