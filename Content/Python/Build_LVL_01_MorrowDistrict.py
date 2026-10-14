"""Build a first-pass playable greybox for the Extraction FPS project.

Run from Unreal Editor's Python console with:
    py "<ProjectDir>/Content/Python/Build_LVL_01_MorrowDistrict.py"

The script intentionally uses engine primitive meshes so the level does not
depend on external art packs.
"""

import unreal


LEVEL_PATH = "/Game/Maps/LVL_01_MorrowDistrict"
MATERIAL_PATH = "/Game/Maps/Materials"


def make_color_material(name, color):
    tools = unreal.AssetToolsHelpers.get_asset_tools()
    path = MATERIAL_PATH + "/" + name
    if unreal.EditorAssetLibrary.does_asset_exist(path):
        return unreal.load_asset(path)

    material = tools.create_asset(
        name, MATERIAL_PATH, unreal.Material, unreal.MaterialFactoryNew()
    )
    expression = unreal.MaterialEditingLibrary.create_material_expression(
        material, unreal.MaterialExpressionVectorParameter, -300, 0
    )
    expression.set_editor_property("parameter_name", "BaseColor")
    expression.set_editor_property("default_value", unreal.LinearColor(*color))
    unreal.MaterialEditingLibrary.connect_material_property(
        expression, "RGB", unreal.MaterialProperty.MP_BASE_COLOR
    )
    unreal.MaterialEditingLibrary.recompile_material(material)
    return material


def spawn_mesh(mesh, label, location, scale, rotation=None, material=None):
    rotation = rotation or unreal.Rotator(0.0, 0.0, 0.0)
    actor = unreal.EditorLevelLibrary.spawn_actor_from_class(
        unreal.StaticMeshActor, unreal.Vector(*location), rotation
    )
    actor.set_actor_label(label)
    component = actor.static_mesh_component
    component.set_static_mesh(mesh)
    component.set_world_scale3d(unreal.Vector(*scale))
    if material:
        component.set_material(0, material)
    return actor


def box(mesh, label, x, y, z, width, depth, height, material, yaw=0.0):
    return spawn_mesh(
        mesh,
        label,
        (x, y, z),
        (width / 100.0, depth / 100.0, height / 100.0),
        unreal.Rotator(0.0, yaw, 0.0),
        material,
    )


def road(mesh, label, x, y, length, width, yaw, asphalt):
    return box(mesh, label, x, y, -15.0, length, width, 30.0, asphalt, yaw)


def wall(mesh, label, x, y, z, length, height, yaw, concrete):
    return box(mesh, label, x, y, z, length, 30.0, height, concrete, yaw)


def building(mesh, label, x, y, width, depth, floors, concrete, interior):
    """Create a hollow, enterable shell with doors centered on two sides."""
    floor_height = 360.0
    height = floors * floor_height
    half_w = width / 2.0
    half_d = depth / 2.0
    box(mesh, label + " / ground", x, y, 0.0, width, depth, 40.0, interior)

    # Front and rear facades use 260 cm door openings for readable routes.
    for side, yy in (("south", y - half_d), ("north", y + half_d)):
        segment = (width - 260.0) / 2.0
        wall(mesh, label + " / " + side + " wall L", x - (width + 260.0) / 4.0, yy, height / 2.0, segment, height, 0.0, concrete)
        wall(mesh, label + " / " + side + " wall R", x + (width + 260.0) / 4.0, yy, height / 2.0, segment, height, 0.0, concrete)

    # Side walls stay solid to make each block read as an urban mass.
    for side, xx in (("west", x - half_w), ("east", x + half_w)):
        wall(mesh, label + " / " + side + " wall", xx, y, height / 2.0, depth, height, 90.0, concrete)

    # Roof slabs are low enough for the template's standard jump to reach.
    box(mesh, label + " / roof", x, y, height + 20.0, width, depth, 40.0, concrete)

    # A few internal partitions create short sightlines and combat cover.
    if width > 2800.0:
        box(mesh, label + " / interior partition", x + width * 0.20, y, 150.0, 30.0, depth * 0.55, 300.0, concrete)


def cover(mesh, label, x, y, width, depth, height, material):
    return box(mesh, label, x, y, height / 2.0, width, depth, height, material)


def main():
    level_editor = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
    actor_editor = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
    if unreal.EditorAssetLibrary.does_asset_exist(LEVEL_PATH):
        if not level_editor.load_level(LEVEL_PATH):
            raise RuntimeError("Could not load the existing blockout level")
        actors = actor_editor.get_all_level_actors()
        if actors and not actor_editor.destroy_actors(actors):
            raise RuntimeError("Could not clear actors from the existing blockout level")
    elif not level_editor.new_level(LEVEL_PATH, False):
        raise RuntimeError("Could not create level: " + LEVEL_PATH)

    cube = unreal.load_asset("/Engine/BasicShapes/Cube.Cube")
    cylinder = unreal.load_asset("/Engine/BasicShapes/Cylinder.Cylinder")
    if not cube or not cylinder:
        raise RuntimeError("Unreal basic shape meshes could not be loaded")

    concrete = make_color_material("M_Blockout_Concrete", (0.24, 0.28, 0.30, 1.0))
    asphalt = make_color_material("M_Blockout_Asphalt", (0.10, 0.12, 0.13, 1.0))
    pavement = make_color_material("M_Blockout_Pavement", (0.38, 0.39, 0.36, 1.0))
    interior = make_color_material("M_Blockout_Interior", (0.30, 0.31, 0.28, 1.0))
    cover_mat = make_color_material("M_Blockout_Cover", (0.42, 0.31, 0.19, 1.0))
    objective_mat = make_color_material("M_Blockout_Objective", (0.08, 0.42, 0.48, 1.0))
    extract_mat = make_color_material("M_Blockout_Extraction", (0.16, 0.50, 0.20, 1.0))

    # A 240 x 200 m playable district; the raised perimeter stops accidental exits.
    box(cube, "Blockout / base", 0.0, 0.0, -100.0, 24000.0, 20000.0, 200.0, pavement)
    for x in (-12000.0, 12000.0):
        box(cube, "Blockout / perimeter west-east", x, 0.0, 400.0, 300.0, 20000.0, 800.0, concrete)
    for y in (-10000.0, 10000.0):
        box(cube, "Blockout / perimeter north-south", 0.0, y, 400.0, 24000.0, 300.0, 800.0, concrete)

    # Asphalt routes connect the insertion street, market junction and extraction.
    road(cube, "Road / insertion street", -5000.0, -7600.0, 14000.0, 1600.0, 0.0, asphalt)
    road(cube, "Road / market street", 0.0, 0.0, 22000.0, 1800.0, 0.0, asphalt)
    road(cube, "Road / north-south avenue", 0.0, 0.0, 19000.0, 1800.0, 90.0, asphalt)
    road(cube, "Road / west service alley", -10000.0, 1000.0, 16500.0, 900.0, 90.0, asphalt)
    road(cube, "Road / east service alley", 10600.0, 1500.0, 15500.0, 1000.0, 90.0, asphalt)
    road(cube, "Road / north extraction approach", 0.0, 7300.0, 22000.0, 1300.0, 0.0, asphalt)

    # Walkable sidewalks parallel to the primary streets.
    for side in (-1.0, 1.0):
        road(cube, "Sidewalk / market " + str(side), 0.0, side * 1150.0, 22000.0, 500.0, 0.0, pavement)
        road(cube, "Sidewalk / avenue " + str(side), side * 1150.0, 0.0, 19000.0, 500.0, 90.0, pavement)

    # Enterable district buildings leave streets and service alleys open.
    building(cube, "Market / west shops", -7000.0, -3400.0, 4200.0, 3600.0, 2, concrete, interior)
    building(cube, "Market / east shops", -2400.0, -3400.0, 3600.0, 3600.0, 2, concrete, interior)
    building(cube, "Clinic / south wing", 3100.0, -3300.0, 4200.0, 3800.0, 2, concrete, interior)
    building(cube, "Apartments / west block", -6900.0, 3600.0, 4000.0, 4200.0, 3, concrete, interior)
    building(cube, "Apartments / mid block", -2700.0, 3900.0, 3300.0, 4200.0, 3, concrete, interior)
    building(cube, "Clinic / north wing", 2900.0, 3500.0, 4000.0, 3800.0, 2, concrete, interior)
    building(cube, "Warehouse / north block", 8100.0, 3500.0, 3600.0, 4200.0, 2, concrete, interior)

    # Waist-high cover shapes support readable first-person firefights.
    for i, (x, y) in enumerate(((-5200, -7000), (-1500, -7000), (2700, -7000), (5200, -2400), (-4600, 2200), (4500, 5400), (9500, 1700))):
        cover(cube, "Cover / road barrier %02d" % (i + 1), x, y, 260.0, 900.0, 120.0, cover_mat)
    for i, (x, y) in enumerate(((-9000, 500), (-7600, 500), (-9000, 7500), (5700, 6000))):
        cover(cube, "Cover / crate stack %02d" % (i + 1), x, y, 180.0, 180.0, 160.0, cover_mat)

    # Mission landmarks are deliberately simple, visible blockout markers.
    box(cube, "Objective / clinic data cache", 3600.0, -1600.0, 70.0, 180.0, 180.0, 140.0, objective_mat)
    box(cube, "Objective / cache plinth", 3600.0, -1600.0, 15.0, 420.0, 420.0, 30.0, concrete)
    spawn_mesh(cylinder, "Extraction / zone marker", (8200.0, 7300.0, 15.0), (8.0, 8.0, 0.30), material=extract_mat)
    cover(cube, "Extraction / shelter", 9200.0, 7300.0, 900.0, 220.0, 240.0, concrete)

    # A simple daylight rig keeps the greybox readable without baked lighting.
    sun = unreal.EditorLevelLibrary.spawn_actor_from_class(
        unreal.DirectionalLight, unreal.Vector(0.0, 0.0, 5000.0), unreal.Rotator(-35.0, -35.0, 0.0)
    )
    sun.set_actor_label("Lighting / directional sun")
    sky = unreal.EditorLevelLibrary.spawn_actor_from_class(
        unreal.SkyLight, unreal.Vector(0.0, 0.0, 4500.0), unreal.Rotator(0.0, 0.0, 0.0)
    )
    sky.set_actor_label("Lighting / sky fill")
    sky.light_component.set_editor_property("mobility", unreal.ComponentMobility.MOVABLE)
    sky.light_component.set_editor_property("source_type", unreal.SkyLightSourceType.SLS_CAPTURED_SCENE)
    sky.light_component.recapture_sky()

    # Spawn faces toward the market street and the first combat space.
    start = unreal.EditorLevelLibrary.spawn_actor_from_class(
        unreal.PlayerStart, unreal.Vector(-9000.0, -7600.0, 120.0), unreal.Rotator(0.0, 0.0, 0.0)
    )
    start.set_actor_label("Insertion / PlayerStart")

    # Keep a single navigation volume over the street and reachable interiors.
    nav = unreal.EditorLevelLibrary.spawn_actor_from_class(
        unreal.NavMeshBoundsVolume, unreal.Vector(0.0, 0.0, 1000.0), unreal.Rotator(0.0, 0.0, 0.0)
    )
    nav.set_actor_label("Navigation / district bounds")
    nav.set_actor_scale3d(unreal.Vector(120.0, 100.0, 20.0))

    level_editor.save_current_level()
    unreal.EditorAssetLibrary.save_directory("/Game/Maps", only_if_is_dirty=False, recursive=True)
    unreal.log("Created LVL_01_MorrowDistrict with urban routes, interior shells, objective, extraction and player start")


main()
