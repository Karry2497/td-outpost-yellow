"""Apply an original high-contrast palette to the existing Morrow District.

Run from the Unreal Editor Python console with:
    py "<ProjectDir>/Content/Python/Apply_Morrow_Stylized_Palette.py"
"""

import unreal


PALETTE = {
    "/Game/Maps/Materials/M_Blockout_Concrete": (0.12, 0.28, 0.34, 1.0),
    "/Game/Maps/Materials/M_Blockout_OutpostMasonry": (0.48, 0.13, 0.035, 1.0),
    "/Game/Maps/Materials/M_Blockout_Asphalt": (0.012, 0.022, 0.05, 1.0),
    "/Game/Maps/Materials/M_Blockout_Pavement": (0.22, 0.36, 0.33, 1.0),
    "/Game/Maps/Materials/M_Blockout_Interior": (0.025, 0.045, 0.08, 1.0),
    "/Game/Maps/Materials/M_Blockout_Cover": (0.58, 0.075, 0.025, 1.0),
    "/Game/Maps/Materials/M_Blockout_Window": (0.01, 0.20, 0.32, 1.0),
    "/Game/Maps/Materials/M_Blockout_RustedMetal": (0.22, 0.055, 0.025, 1.0),
}

ZOMBIE_MATERIAL = "/Game/Maps/Materials/M_Zombie_Yellow"
ZOMBIE_YELLOW = (1.0, 0.72, 0.015, 1.0)


def make_zombie_material():
    """Make a flat yellow material that supports animated skeletal meshes."""
    material = unreal.load_asset(ZOMBIE_MATERIAL)
    if not material:
        material = unreal.AssetToolsHelpers.get_asset_tools().duplicate_asset(
            "M_Zombie_Yellow",
            "/Game/Maps/Materials",
            unreal.load_asset("/Game/Maps/Materials/M_Blockout_Cover"),
        )
    if not material:
        raise RuntimeError("Could not create the skeletal zombie material")

    parameter = next(
        (
            expression
            for expression in unreal.MaterialEditingLibrary.get_material_expressions(material)
            if isinstance(expression, unreal.MaterialExpressionVectorParameter)
            and expression.get_editor_property("parameter_name") == "BaseColor"
        ),
        None,
    )
    if not parameter:
        raise RuntimeError("Zombie material has no BaseColor vector parameter")

    parameter.set_editor_property("default_value", unreal.LinearColor(*ZOMBIE_YELLOW))
    material.set_editor_property("used_with_skeletal_mesh", True)
    unreal.MaterialEditingLibrary.recompile_material(material)
    unreal.EditorAssetLibrary.save_loaded_asset(material)
    return material


def main():
    updated = []
    missing = []
    for asset_path, color in PALETTE.items():
        material = unreal.load_asset(asset_path)
        if not material:
            missing.append(asset_path)
            unreal.log_error("Palette material not found: " + asset_path)
            continue

        parameter = None
        for expression in unreal.MaterialEditingLibrary.get_material_expressions(material):
            if isinstance(expression, unreal.MaterialExpressionVectorParameter):
                if expression.get_editor_property("parameter_name") == "BaseColor":
                    parameter = expression
                    break
        if not parameter:
            missing.append(asset_path + " (BaseColor vector parameter)")
            unreal.log_error("BaseColor parameter not found: " + asset_path)
            continue

        parameter.set_editor_property("default_value", unreal.LinearColor(*color))
        unreal.MaterialEditingLibrary.recompile_material(material)
        unreal.EditorAssetLibrary.save_loaded_asset(material)
        updated.append(asset_path.rsplit("/", 1)[-1])

    unreal.log("Applied Morrow stylized palette to: " + ", ".join(updated))
    zombie_material = make_zombie_material()
    unreal.log("Created skeletal-ready yellow zombie material: " + zombie_material.get_path_name())
    if missing:
        raise RuntimeError("Palette not fully applied; unresolved assets: " + ", ".join(missing))


main()
