"""Build a first-pass playable greybox for the Extraction FPS project.

Run from Unreal Editor's Python console with:
    py "<ProjectDir>/Content/Python/Build_LVL_01_MorrowDistrict.py"

The script intentionally uses engine primitive meshes so the level does not
depend on external art packs.
"""

import math

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


def wall(mesh, label, x, y, z, length, height, yaw, concrete, thickness=30.0):
    return box(mesh, label, x, y, z, length, thickness, height, concrete, yaw)


def building(mesh, label, x, y, width, depth, floors, concrete, interior, glass):
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

    # Dark glass panes are deliberately simple blockout windows. Their shallow
    # boxes sit on the four facades and make every storey readable at a distance.
    for floor in range(floors):
        window_z = floor * floor_height + 205.0
        for side, yy in (("south", y - half_d - 12.0), ("north", y + half_d + 12.0)):
            for index in range(3):
                wx = x + (index - 1) * min(760.0, width / 4.0)
                if floor == 0 and index == 1:
                    continue  # keep the ground-floor doorway clear
                box(mesh, "%s / window %s F%d %d" % (label, side, floor + 1, index + 1), wx, yy, window_z, 210.0, 24.0, 150.0, glass)
        for side, xx, yaw in (("west", x - half_w - 12.0, 90.0), ("east", x + half_w + 12.0, 90.0)):
            for index in range(2):
                wy = y + (index - 0.5) * min(900.0, depth / 3.0)
                box(mesh, "%s / window %s F%d %d" % (label, side, floor + 1, index + 1), xx, wy, window_z, 210.0, 24.0, 150.0, glass, yaw)

    # A few internal partitions create short sightlines and combat cover.
    if width > 2800.0:
        box(mesh, label + " / interior partition", x + width * 0.20, y, 150.0, 30.0, depth * 0.55, 300.0, concrete)


def cover(mesh, label, x, y, width, depth, height, material):
    return box(mesh, label, x, y, height / 2.0, width, depth, height, material)


def wrecked_car(mesh, label, x, y, yaw, rust, glass, concrete):
    """Create a stationary, visibly damaged car from simple blockout meshes."""
    box(mesh, label + " / bent body", x, y, 72.0, 500.0, 220.0, 120.0, rust, yaw)
    box(mesh, label + " / crushed cabin", x + math.cos(math.radians(yaw)) * 15.0, y + math.sin(math.radians(yaw)) * 15.0, 174.0, 245.0, 175.0, 88.0, rust, yaw - 5.0)
    box(mesh, label + " / shattered windscreen", x + math.cos(math.radians(yaw)) * 105.0, y + math.sin(math.radians(yaw)) * 105.0, 177.0, 105.0, 145.0, 12.0, glass, yaw)

    for dx, dy in ((-165.0, -125.0), (-165.0, 125.0), (165.0, -125.0), (165.0, 125.0)):
        angle = math.radians(yaw)
        wheel_x = x + dx * math.cos(angle) - dy * math.sin(angle)
        wheel_y = y + dx * math.sin(angle) + dy * math.cos(angle)
        box(mesh, label + " / damaged tire", wheel_x, wheel_y, 48.0, 104.0, 70.0, 88.0, concrete, yaw)

    # A twisted bonnet and scattered concrete chunks sell the abandoned state.
    box(mesh, label + " / folded bonnet", x - math.cos(math.radians(yaw)) * 175.0, y - math.sin(math.radians(yaw)) * 175.0, 112.0, 180.0, 215.0, 32.0, concrete, yaw + 12.0)


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
    if not cube:
        raise RuntimeError("Unreal basic shape meshes could not be loaded")

    concrete = make_color_material("M_Blockout_Concrete", (0.24, 0.28, 0.30, 1.0))
    asphalt = make_color_material("M_Blockout_Asphalt", (0.10, 0.12, 0.13, 1.0))
    pavement = make_color_material("M_Blockout_Pavement", (0.38, 0.39, 0.36, 1.0))
    interior = make_color_material("M_Blockout_Interior", (0.30, 0.31, 0.28, 1.0))
    cover_mat = make_color_material("M_Blockout_Cover", (0.42, 0.31, 0.19, 1.0))
    objective_mat = make_color_material("M_Blockout_Objective", (0.08, 0.42, 0.48, 1.0))
    glass_mat = make_color_material("M_Blockout_Window", (0.035, 0.10, 0.14, 1.0))
    rust = make_color_material("M_Blockout_RustedMetal", (0.22, 0.12, 0.075, 1.0))

    # A 240 x 200 m playable district; the raised perimeter stops accidental exits.
    box(cube, "Blockout / base", 0.0, 0.0, -100.0, 24000.0, 20000.0, 200.0, pavement)
    for x in (-12000.0, 12000.0):
        box(cube, "Blockout / perimeter west-east", x, 0.0, 400.0, 300.0, 20000.0, 800.0, concrete)
    for y in (-10000.0, 10000.0):
        box(cube, "Blockout / perimeter north-south", 0.0, y, 400.0, 24000.0, 300.0, 800.0, concrete)

    # The former streets now stop at the outpost walls. There is no road through
    # the courtyard and no fourth road arm behind the connected rear buildings.
    compound_half = 1500.0
    road(cube, "Road / west dead-end approach", -6825.0, 0.0, 10350.0, 1800.0, 0.0, asphalt)
    road(cube, "Road / east dead-end approach", 6825.0, 0.0, 10350.0, 1800.0, 0.0, asphalt)
    road(cube, "Road / south dead-end approach", 0.0, -5825.0, 8350.0, 1800.0, 90.0, asphalt)

    # Three-storey building masses meet the compound at every rear and side
    # edge, so there is no alley for zombies or players to use as a bypass.
    building(cube, "District / northwest connected block", -6825.0, 5825.0, 10350.0, 8350.0, 3, concrete, interior, glass_mat)
    building(cube, "District / northeast connected block", 6825.0, 5825.0, 10350.0, 8350.0, 3, concrete, interior, glass_mat)
    building(cube, "District / southwest connected block", -6825.0, -5825.0, 10350.0, 8350.0, 3, concrete, interior, glass_mat)
    building(cube, "District / southeast connected block", 6825.0, -5825.0, 10350.0, 8350.0, 3, concrete, interior, glass_mat)
    building(cube, "Outpost / attached rear support building", 0.0, 2550.0, 3300.0, 1800.0, 3, concrete, interior, glass_mat)

    # Abandoned wrecks are static blockout props; no vehicles are in operation.
    wrecked_car(cube, "Wreck / west approach", -5800.0, 220.0, 0.0, rust, glass_mat, concrete)
    wrecked_car(cube, "Wreck / east approach", 5800.0, -240.0, 0.0, rust, glass_mat, concrete)
    wrecked_car(cube, "Wreck / south approach", -260.0, -5650.0, 90.0, rust, glass_mat, concrete)

    # The open outpost occupies the old intersection and blocks all three roads.
    # Its complete wall seals the courtyard; connected buildings close the flanks.
    wall_height = 620.0
    wall_thickness = 300.0
    wall(cube, "Outpost / perimeter south", 0.0, -compound_half, wall_height / 2.0, compound_half * 2.0, wall_height, 0.0, concrete, wall_thickness)
    wall(cube, "Outpost / perimeter north", 0.0, compound_half, wall_height / 2.0, compound_half * 2.0, wall_height, 0.0, concrete, wall_thickness)
    wall(cube, "Outpost / perimeter west", -compound_half, 0.0, wall_height / 2.0, compound_half * 2.0, wall_height, 90.0, concrete, wall_thickness)
    wall(cube, "Outpost / perimeter east", compound_half, 0.0, wall_height / 2.0, compound_half * 2.0, wall_height, 90.0, concrete, wall_thickness)

    # A connected firing walk reaches the three road-facing walls. The rear
    # section joins the ring to keep the walk connected but has no firing merlons.
    walk_width = 800.0
    walk_thickness = 80.0
    walk_top = wall_height + walk_thickness
    for side, y in (("south", -compound_half), ("north", compound_half)):
        box(cube, "Outpost / firing walk " + side, 0.0, y, wall_height + walk_thickness / 2.0, compound_half * 2.0 + walk_width, walk_width, walk_thickness, concrete)
    for side, x in (("west", -compound_half), ("east", compound_half)):
        box(cube, "Outpost / firing walk " + side, x, 0.0, wall_height + walk_thickness / 2.0, compound_half * 2.0 + walk_width, walk_width, walk_thickness, concrete, 90.0)

    firing_positions = (-1200.0, -600.0, 0.0, 600.0, 1200.0)
    for side, y in (("south", -compound_half - walk_width / 2.0 + 60.0),):
        for pos in firing_positions:
            if pos == 0.0:
                continue  # leave the south stair clear
            box(cube, "Outpost / %s firing merlon %d" % (side, int(pos)), pos, y, walk_top + 80.0, 260.0, 120.0, 160.0, concrete)
    for side, x in (("west", -compound_half - walk_width / 2.0 + 60.0), ("east", compound_half + walk_width / 2.0 - 60.0)):
        for pos in firing_positions:
            if pos == 0.0:
                continue  # leave room for the side staircase
            box(cube, "Outpost / %s firing merlon %d" % (side, int(pos)), x, pos, walk_top + 80.0, 260.0, 120.0, 160.0, concrete, 90.0)

    # Three stepped approaches lead from the open courtyard to the firing walk.
    stair_count = 12
    stair_run = (compound_half * 2.0) / stair_count
    stair_rise = walk_top / stair_count
    for i in range(stair_count):
        step_center_z = stair_rise * (i + 0.5)
        inward = stair_run * (i + 0.5)
        box(cube, "Outpost / stair south %02d" % (i + 1), 0.0, -inward, step_center_z, 380.0, stair_run, stair_rise, concrete)
        box(cube, "Outpost / stair west %02d" % (i + 1), -inward, 0.0, step_center_z, stair_run, 380.0, stair_rise, concrete)
        box(cube, "Outpost / stair east %02d" % (i + 1), inward, 0.0, step_center_z, stair_run, 380.0, stair_rise, concrete)

    # Road debris and waist-high cover sit on the abandoned approaches.
    for i, (x, y) in enumerate(((-2800, 720), (-4400, -650), (3000, -700), (4600, 640), (-650, -3300), (600, -4700))):
        cover(cube, "Cover / road rubble %02d" % (i + 1), x, y, 420.0, 360.0, 180.0, cover_mat)

    # A visible landmark marks the outpost as the zombie-control checkpoint.
    box(cube, "Objective / zombie count console", 0.0, 0.0, 70.0, 180.0, 180.0, 140.0, objective_mat)
    box(cube, "Objective / console plinth", 0.0, 0.0, 15.0, 420.0, 420.0, 30.0, concrete)

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

    # Start safely inside the courtyard so the player can reach the three stairs.
    start = unreal.EditorLevelLibrary.spawn_actor_from_class(
        unreal.PlayerStart, unreal.Vector(0.0, -600.0, 120.0), unreal.Rotator(0.0, -90.0, 0.0)
    )
    start.set_actor_label("Insertion / PlayerStart")

    # Keep a single navigation volume over the street and reachable interiors.
    nav = unreal.EditorLevelLibrary.spawn_actor_from_class(
        unreal.NavMeshBoundsVolume, unreal.Vector(0.0, 0.0, 1000.0), unreal.Rotator(0.0, 0.0, 0.0)
    )
    nav.set_actor_label("Navigation / district bounds")
    nav.set_actor_scale3d(unreal.Vector(120.0, 100.0, 20.0))

    level_editor.save_current_level()
    unreal.EditorAssetLibrary.save_directory("/Game/Maps", only_if_is_dirty=True, recursive=True)
    unreal.log("Created LVL_01_MorrowDistrict with a road-blocking outpost, three dead-end approaches, wrecked cars, three firing-wall stairs and buildings connected around the rear and sides")


main()
