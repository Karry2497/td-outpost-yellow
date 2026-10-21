"""Build the stylized, playable first level for the Extraction TD project.

Run from Unreal Editor's Python console with:
    py "<ProjectDir>/Content/Python/Build_LVL_01_MorrowDistrict.py"

The script intentionally uses engine primitive meshes so the level does not
depend on external art packs.
"""

import math

import unreal


LEVEL_PATH = "/Game/Maps/LVL_01_MorrowDistrict_Walled"
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


def build_pad(mesh, label, x, y, pad_material, mark_material):
    """Mark a small, non-blocking shoulder spot for one defense deployable."""
    box(mesh, label + " / base", x, y, 14.0, 300.0, 300.0, 28.0, pad_material)
    box(mesh, label + " / cross mark", x, y, 30.0, 190.0, 28.0, 4.0, mark_material)
    box(mesh, label + " / cross mark", x, y, 30.0, 28.0, 190.0, 4.0, mark_material)


def wall(mesh, label, x, y, z, length, height, yaw, concrete, thickness=30.0):
    return box(mesh, label, x, y, z, length, thickness, height, concrete, yaw)


def building(mesh, label, x, y, width, depth, floors, facade, interior, glass, trim):
    """Create a hollow, enterable shell with doors centered on two sides."""
    floor_height = 360.0
    height = floors * floor_height
    half_w = width / 2.0
    half_d = depth / 2.0
    box(mesh, label + " / ground", x, y, 0.0, width, depth, 40.0, interior)

    # Front and rear facades use 260 cm door openings for readable routes.
    for side, yy in (("south", y - half_d), ("north", y + half_d)):
        segment = (width - 260.0) / 2.0
        wall(mesh, label + " / " + side + " wall L", x - (width + 260.0) / 4.0, yy, height / 2.0, segment, height, 0.0, facade)
        wall(mesh, label + " / " + side + " wall R", x + (width + 260.0) / 4.0, yy, height / 2.0, segment, height, 0.0, facade)

    # Side walls stay solid to make each block read as an urban mass.
    for side, xx in (("west", x - half_w), ("east", x + half_w)):
        wall(mesh, label + " / " + side + " wall", xx, y, height / 2.0, depth, height, 90.0, facade)

    # Roof slabs are low enough for the template's standard jump to reach.
    box(mesh, label + " / roof", x, y, height + 20.0, width, depth, 40.0, facade)
    # Broad corner pilasters and thin floor bands make each storey readable
    # from the raised camera and give each city block its own bold color.
    for sx in (-1.0, 1.0):
        for sy in (-1.0, 1.0):
            box(mesh, label + " / corner trim", x + sx * (half_w - 28.0), y + sy * (half_d - 28.0), height / 2.0, 58.0, 58.0, height, trim)
    for floor in range(floors):
        band_z = (floor + 1) * floor_height - 30.0
        for side_y in (-1.0, 1.0):
            box(mesh, label + " / floor band", x, y + side_y * (half_d + 18.0), band_z, width - 40.0, 26.0, 24.0, trim)
        for side_x in (-1.0, 1.0):
            box(mesh, label + " / floor band", x + side_x * (half_w + 18.0), y, band_z, 26.0, depth - 40.0, 24.0, trim)
    # A light roof edge makes the skyline easy to parse without adding more
    # gameplay cover or changing the open courtyard.
    for side_y in (-1.0, 1.0):
        box(mesh, label + " / roof edge", x, y + side_y * (half_d - 36.0), height + 48.0, width, 36.0, 16.0, trim)
    for side_x in (-1.0, 1.0):
        box(mesh, label + " / roof edge", x + side_x * (half_w - 36.0), y, height + 48.0, 36.0, depth, 16.0, trim)

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
                box(mesh, "%s / window sill %s" % (label, side), wx, yy, window_z - 82.0, 250.0, 36.0, 18.0, trim)
        for side, xx, yaw in (("west", x - half_w - 12.0, 90.0), ("east", x + half_w + 12.0, 90.0)):
            for index in range(2):
                wy = y + (index - 0.5) * min(900.0, depth / 3.0)
                box(mesh, "%s / window %s F%d %d" % (label, side, floor + 1, index + 1), xx, wy, window_z, 210.0, 24.0, 150.0, glass, yaw)
                box(mesh, "%s / window sill %s" % (label, side), xx, wy, window_z - 82.0, 250.0, 36.0, 18.0, trim, yaw)

    # A few internal partitions create short sightlines and combat cover.
    if width > 2800.0:
        box(mesh, label + " / interior partition", x + width * 0.20, y, 150.0, 30.0, depth * 0.55, 300.0, facade)


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
    extraction_gate_class = unreal.load_class(None, "/Script/extractionFPS.TDExtractionGate")
    if not extraction_gate_class:
        raise RuntimeError("Compile the extractionFPS module before building the level gates")

    concrete = make_color_material("M_Stylized_Concrete", (0.68, 0.72, 0.68, 1.0))
    outpost_mat = make_color_material("M_Stylized_OutpostMasonry", (0.40, 0.29, 0.20, 1.0))
    asphalt = make_color_material("M_Stylized_Asphalt", (0.105, 0.15, 0.17, 1.0))
    pavement = make_color_material("M_Stylized_Pavement", (0.62, 0.66, 0.61, 1.0))
    build_pad_mat = make_color_material("M_Stylized_BuildPad", (0.96, 0.40, 0.055, 1.0))
    build_pad_mark = make_color_material("M_Stylized_BuildPadMark", (1.0, 0.83, 0.27, 1.0))
    extraction_gate_mat = make_color_material("M_Stylized_ExtractionGate", (0.96, 0.22, 0.08, 1.0))
    interior = make_color_material("M_Stylized_Interior", (0.34, 0.38, 0.36, 1.0))
    cover_mat = make_color_material("M_Stylized_Cover", (0.44, 0.31, 0.20, 1.0))
    glass_mat = make_color_material("M_Stylized_Window", (0.045, 0.22, 0.26, 1.0))
    rust = make_color_material("M_Stylized_RustedMetal", (0.22, 0.15, 0.12, 1.0))
    facade_blue = make_color_material("M_Stylized_FacadeBlue", (0.18, 0.48, 0.60, 1.0))
    facade_coral = make_color_material("M_Stylized_FacadeCoral", (0.88, 0.38, 0.27, 1.0))
    facade_ochre = make_color_material("M_Stylized_FacadeOchre", (0.84, 0.63, 0.30, 1.0))
    facade_mint = make_color_material("M_Stylized_FacadeMint", (0.33, 0.63, 0.49, 1.0))
    trim_blue = make_color_material("M_Stylized_TrimBlue", (0.10, 0.31, 0.39, 1.0))
    trim_coral = make_color_material("M_Stylized_TrimCoral", (0.58, 0.22, 0.17, 1.0))
    trim_ochre = make_color_material("M_Stylized_TrimOchre", (0.57, 0.39, 0.16, 1.0))
    trim_mint = make_color_material("M_Stylized_TrimMint", (0.17, 0.39, 0.30, 1.0))
    road_mark = make_color_material("M_Stylized_RoadMark", (0.95, 0.76, 0.31, 1.0))
    helipad = make_color_material("M_Stylized_Helipad", (0.30, 0.43, 0.40, 1.0))
    heli_paint = make_color_material("M_Stylized_HelicopterPaint", (0.13, 0.39, 0.35, 1.0))

    # A 240 x 200 m playable district; the raised perimeter stops accidental exits.
    box(cube, "Blockout / base", 0.0, 0.0, -100.0, 24000.0, 20000.0, 200.0, pavement)
    for x in (-12000.0, 12000.0):
        box(cube, "Blockout / perimeter west-east", x, 0.0, 400.0, 300.0, 20000.0, 800.0, concrete)
    for y in (10000.0,):
        box(cube, "Blockout / perimeter north-south", 0.0, y, 400.0, 24000.0, 300.0, 800.0, concrete)
    # A gated opening in the south city boundary lines up with the extraction road.
    for x in (-6650.0, 6650.0):
        box(cube, "Blockout / perimeter south gate flank", x, -10000.0, 400.0, 10700.0, 300.0, 800.0, concrete)
    outer_gate = unreal.EditorLevelLibrary.spawn_actor_from_class(
        extraction_gate_class, unreal.Vector(0.0, -10000.0, 400.0), unreal.Rotator(0.0, 0.0, 0.0)
    )
    outer_gate.set_actor_label("Extraction / closed south city gate")
    outer_gate.set_actor_scale3d(unreal.Vector(26.0, 3.0, 8.0))
    outer_gate.get_component_by_class(unreal.StaticMeshComponent).set_material(0, extraction_gate_mat)

    # The former streets now stop at the outpost walls. There is no road through
    # the courtyard and no fourth road arm behind the connected rear buildings.
    compound_half = 1500.0
    walk_width = 800.0
    road(cube, "Road / west dead-end approach", -6825.0, 0.0, 10350.0, 1800.0, 0.0, asphalt)
    road(cube, "Road / east dead-end approach", 6825.0, 0.0, 10350.0, 1800.0, 0.0, asphalt)
    road(cube, "Road / south dead-end approach", 0.0, -5825.0, 8350.0, 1800.0, 90.0, asphalt)
    # Faded center dashes make each of the three zombie lanes easy to read.
    for i, x in enumerate(range(-10800, -2600, 1000)):
        box(cube, "Road marking / west dash %02d" % i, x, 0.0, 2.0, 430.0, 34.0, 4.0, road_mark)
    for i, x in enumerate(range(2600, 10801, 1000)):
        box(cube, "Road marking / east dash %02d" % i, x, 0.0, 2.0, 430.0, 34.0, 4.0, road_mark)
    for i, y in enumerate(range(-10000, -2500, 1000)):
        box(cube, "Road marking / south dash %02d" % i, 0.0, y, 2.0, 34.0, 430.0, 4.0, road_mark)

    # Six clear defense pads sit on the shoulders, keeping all three zombie
    # lanes open while giving players more useful tower coverage choices.
    tower_pad_locations = (
        ("west approach A", -6400.0, -1220.0),
        ("west approach B", -4200.0, 1220.0),
        ("east approach A", 6400.0, 1220.0),
        ("east approach B", 4200.0, -1220.0),
        ("south approach A", -1220.0, -6400.0),
        ("south approach B", 1220.0, -4200.0),
    )
    for label, pad_x, pad_y in tower_pad_locations:
        build_pad(cube, "Defense build pad / " + label, pad_x, pad_y, build_pad_mat, build_pad_mark)

    # The extraction staging area sits beyond the south edge of town. Extend
    # the abandoned street into a separate apron, with a visible landing mark
    # and a deliberately simple helicopter silhouette for the greybox.
    box(cube, "Extraction / outer staging ground", 0.0, -14000.0, -100.0, 8000.0, 8000.0, 200.0, pavement)
    road(cube, "Extraction / access road beyond city", 0.0, -12000.0, 7600.0, 1800.0, 90.0, asphalt)
    box(cube, "Extraction / helicopter landing pad", 0.0, -15000.0, 30.0, 6000.0, 6000.0, 60.0, helipad)
    for x in (-2900.0, 2900.0):
        box(cube, "Extraction / pad edge", x, -15000.0, 80.0, 120.0, 6000.0, 40.0, cover_mat)
    for y in (-17900.0, -12100.0):
        box(cube, "Extraction / pad edge", 0.0, y, 80.0, 6000.0, 120.0, 40.0, cover_mat)
    # H-shaped touchdown mark; reserve its eastern half for the parked aircraft.
    for x in (-1700.0, -100.0):
        box(cube, "Extraction / H landing mark leg", x, -15000.0, 72.0, 180.0, 1500.0, 24.0, build_pad_mark)
    box(cube, "Extraction / H landing mark bar", -900.0, -15000.0, 72.0, 1500.0, 180.0, 24.0, build_pad_mark)
    # Large corner chevrons and a boundary stripe make the extraction pad pop.
    for sx in (-1.0, 1.0):
        for sy in (-1.0, 1.0):
            box(cube, "Extraction / corner marker", sx * 2550.0, -15000.0 + sy * 2550.0, 74.0, 360.0, 100.0, 24.0, extraction_gate_mat, yaw=45.0 * sx * sy)
    # The helicopter faces south. The separate meshes keep the landing zone
    # editable until a licensed final vehicle model is selected.
    heli_x = 1300.0
    box(cube, "Extraction / helicopter fuselage", heli_x, -15500.0, 260.0, 500.0, 1000.0, 350.0, heli_paint)
    box(cube, "Extraction / helicopter nose", heli_x, -16450.0, 250.0, 430.0, 350.0, 260.0, heli_paint)
    box(cube, "Extraction / helicopter cockpit glass", heli_x, -16550.0, 285.0, 350.0, 100.0, 180.0, glass_mat)
    box(cube, "Extraction / helicopter tail boom", heli_x, -14300.0, 220.0, 140.0, 800.0, 140.0, cover_mat)
    box(cube, "Extraction / helicopter tail fin", heli_x, -14000.0, 330.0, 80.0, 180.0, 260.0, cover_mat)
    box(cube, "Extraction / helicopter rotor mast", heli_x, -15500.0, 470.0, 45.0, 45.0, 200.0, rust)
    box(cube, "Extraction / helicopter main rotor A", heli_x, -15500.0, 580.0, 2600.0, 110.0, 24.0, rust)
    box(cube, "Extraction / helicopter main rotor B", heli_x, -15500.0, 585.0, 110.0, 2600.0, 24.0, rust)
    for x in (heli_x - 520.0, heli_x + 520.0):
        box(cube, "Extraction / helicopter landing skid", x, -15500.0, 90.0, 35.0, 1200.0, 30.0, rust)

    # Three-storey blocks seal the district around the compound, but their
    # inner corners align with the outer edge of the firing walk. This keeps
    # the west/east walls and the connected walk fully exposed and traversable.
    block_inner_edge = compound_half + walk_width / 2.0
    block_width = 12000.0 - block_inner_edge
    block_depth = 10000.0 - block_inner_edge
    block_x = (12000.0 + block_inner_edge) / 2.0
    block_y = (10000.0 + block_inner_edge) / 2.0
    building(cube, "District / northwest connected block", -block_x, block_y, block_width, block_depth, 3, facade_blue, interior, glass_mat, trim_blue)
    building(cube, "District / northeast connected block", block_x, block_y, block_width, block_depth, 3, facade_ochre, interior, glass_mat, trim_ochre)
    building(cube, "District / southwest connected block", -block_x, -block_y, block_width, block_depth, 3, facade_mint, interior, glass_mat, trim_mint)
    building(cube, "District / southeast connected block", block_x, -block_y, block_width, block_depth, 3, facade_coral, interior, glass_mat, trim_coral)
    rear_depth = 2000.0
    building(cube, "Outpost / attached rear support building", 0.0, block_inner_edge + rear_depth / 2.0, width=3800.0, depth=rear_depth, floors=3, facade=facade_blue, interior=interior, glass=glass_mat, trim=trim_blue)

    # Abandoned wrecks are static blockout props; no vehicles are in operation.
    wrecked_car(cube, "Wreck / west approach", -5800.0, 220.0, 0.0, rust, glass_mat, concrete)
    wrecked_car(cube, "Wreck / east approach", 5800.0, -240.0, 0.0, rust, glass_mat, concrete)
    wrecked_car(cube, "Wreck / south approach", -260.0, -5650.0, 90.0, rust, glass_mat, concrete)

    # The open outpost occupies the old intersection and blocks all three roads.
    # Its complete wall seals the courtyard; connected buildings close the flanks.
    wall_height = 620.0
    # Fill the complete wall-top walk footprint down to ground level. A narrow
    # wall under a wide walk left a sheltered cavity where zombies could hide.
    wall_thickness = walk_width
    # The southern wall keeps the same protected firing edge but includes a
    # narrow, barred evacuation doorway that opens after the final wave.
    south_gate_width = 320.0
    south_segment = (compound_half * 2.0 - south_gate_width) / 2.0
    for side, x in (("west", -(south_gate_width / 2.0 + south_segment / 2.0)), ("east", south_gate_width / 2.0 + south_segment / 2.0)):
        wall(cube, "Outpost / perimeter south " + side, x, -compound_half, wall_height / 2.0, south_segment, wall_height, 0.0, outpost_mat, wall_thickness)
    box(cube, "Outpost / south doorway lintel", 0.0, -compound_half, wall_height - 120.0, south_gate_width, wall_thickness, 240.0, outpost_mat)
    outpost_gate = unreal.EditorLevelLibrary.spawn_actor_from_class(
        extraction_gate_class, unreal.Vector(0.0, -compound_half, 190.0), unreal.Rotator(0.0, 0.0, 0.0)
    )
    outpost_gate.set_actor_label("Extraction / closed outpost gate")
    outpost_gate.set_actor_scale3d(unreal.Vector(south_gate_width / 100.0, wall_thickness / 100.0, 3.8))
    outpost_gate.get_component_by_class(unreal.StaticMeshComponent).set_material(0, extraction_gate_mat)
    wall(cube, "Outpost / perimeter north", 0.0, compound_half, wall_height / 2.0, compound_half * 2.0, wall_height, 0.0, outpost_mat, wall_thickness)
    # Define the long side walls directly in world X/Y dimensions. This keeps
    # their 30 m runs along Y and avoids relying on rotated component scaling.
    box(cube, "Outpost / perimeter west", -compound_half, 0.0, wall_height / 2.0, wall_thickness, compound_half * 2.0, wall_height, outpost_mat)
    box(cube, "Outpost / perimeter east", compound_half, 0.0, wall_height / 2.0, wall_thickness, compound_half * 2.0, wall_height, outpost_mat)

    # A connected firing walk reaches the three road-facing walls. The rear
    # section joins the ring to keep the walk connected but has no firing merlons.
    walk_thickness = 80.0
    walk_top = wall_height + walk_thickness
    for side, y in (("south", -compound_half), ("north", compound_half)):
        box(cube, "Outpost / firing walk " + side, 0.0, y, wall_height + walk_thickness / 2.0, compound_half * 2.0 + walk_width, walk_width, walk_thickness, outpost_mat)
    for side, x in (("west", -compound_half), ("east", compound_half)):
        box(cube, "Outpost / firing walk " + side, x, 0.0, wall_height + walk_thickness / 2.0, walk_width, compound_half * 2.0 + walk_width, walk_thickness, outpost_mat)

    # Fill the four ground-level corner gaps between wall ends and buildings.
    # These pads meet the firing walk at its outer corners without covering it.
    corner_fill = walk_width / 2.0
    for sx in (-1.0, 1.0):
        for sy in (-1.0, 1.0):
            box(
                cube,
                "Outpost / sealed corner %s %s" % ("east" if sx > 0 else "west", "north" if sy > 0 else "south"),
                sx * (compound_half + corner_fill / 2.0),
                sy * (compound_half + corner_fill / 2.0),
                walk_top / 2.0,
                corner_fill,
                corner_fill,
                walk_top,
                outpost_mat,
            )

    # Keep the road-facing firing edges completely open. High merlons blocked
    # the player's aim from the wall-top walk; the continuous wall below forms
    # the outpost perimeter without covering the elevated firing positions.

    # Smaller, more natural treads lead from separate courtyard lanes to each
    # firing wall. Keeping them offset leaves the middle of the courtyard open.
    stair_count = 40
    stair_width = 180.0
    # Stop each stair at the inner face of the now-solid wall instead of
    # running into its filled footprint.
    stair_run = (compound_half - wall_thickness / 2.0) / stair_count
    stair_rise = walk_top / stair_count
    for i in range(stair_count):
        step_center_z = stair_rise * (i + 0.5)
        inward = stair_run * (i + 0.5)
        box(cube, "Outpost / stair south %02d" % (i + 1), -900.0, -inward, step_center_z, stair_width, stair_run, stair_rise, outpost_mat)
        box(cube, "Outpost / stair west %02d" % (i + 1), -inward, 900.0, step_center_z, stair_run, stair_width, stair_rise, outpost_mat)
        box(cube, "Outpost / stair east %02d" % (i + 1), inward, -900.0, step_center_z, stair_run, stair_width, stair_rise, outpost_mat)

    # Road debris and waist-high cover sit on the abandoned approaches.
    for i, (x, y) in enumerate(((-2800, 720), (-4400, -650), (3000, -700), (4600, 640), (-650, -3300), (600, -4700))):
        cover(cube, "Cover / road rubble %02d" % (i + 1), x, y, 420.0, 360.0, 180.0, cover_mat)

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
    unreal.log("Created LVL_01_MorrowDistrict with five defense waves, gated southern extraction route, helicopter landing zone, three approaches and the walled outpost")


main()
