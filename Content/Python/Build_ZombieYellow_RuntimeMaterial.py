"""Create a clean UE 5.8 skeletal-mesh material for the yellow zombie."""

import unreal


PACKAGE = "/Game/Maps/Materials"
ASSET_PATH = PACKAGE + "/M_Zombie_StylizedYellow_Runtime"
ALBEDO_PATH = "/Game/Characters/Enemies/YellowZombie/Textures/T_SZombie_Variant_1_A_Albedo"

material = unreal.load_asset(ASSET_PATH)
if not material:
    material = unreal.AssetToolsHelpers.get_asset_tools().create_asset(
        "M_Zombie_StylizedYellow_Runtime",
        PACKAGE,
        unreal.Material,
        unreal.MaterialFactoryNew(),
    )
albedo = unreal.load_asset(ALBEDO_PATH)
if not material or not albedo:
    raise RuntimeError("Could not load or create the zombie material assets")

editing = unreal.MaterialEditingLibrary
texture = editing.create_material_expression(
    material, unreal.MaterialExpressionTextureSample, -600, 0
)
texture.set_editor_property("texture", albedo)
gray = editing.create_material_expression(
    material, unreal.MaterialExpressionDesaturation, -350, 0
)
editing.connect_material_expressions(texture, "RGB", gray, "Input")

tint = editing.create_material_expression(
    material, unreal.MaterialExpressionVectorParameter, -350, 250
)
tint.set_editor_property("parameter_name", "ZombieYellow")
tint.set_editor_property("default_value", unreal.LinearColor(1.0, 0.72, 0.015, 1.0))

multiply = editing.create_material_expression(
    material, unreal.MaterialExpressionMultiply, -50, 0
)
editing.connect_material_expressions(gray, "Output", multiply, "A")
editing.connect_material_expressions(tint, "RGB", multiply, "B")
editing.connect_material_property(
    multiply, "", unreal.MaterialProperty.MP_BASE_COLOR
)
material.set_editor_property("used_with_skeletal_mesh", True)
editing.recompile_material(material)
unreal.EditorAssetLibrary.save_loaded_asset(material)
unreal.log("Created and saved M_Zombie_StylizedYellow_Runtime")
