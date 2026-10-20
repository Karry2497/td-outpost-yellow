"""Apply an original high-contrast palette to the existing Morrow District.

Run from the Unreal Editor Python console with:
    py "<ProjectDir>/Content/Python/Apply_Morrow_Stylized_Palette.py"
"""

import unreal


PALETTE = {
    "/Game/Maps/Materials/M_Blockout_Concrete": (0.30, 0.49, 0.57, 1.0),
    "/Game/Maps/Materials/M_Blockout_OutpostMasonry": (0.88, 0.29, 0.055, 1.0),
    "/Game/Maps/Materials/M_Blockout_Asphalt": (0.025, 0.045, 0.085, 1.0),
    "/Game/Maps/Materials/M_Blockout_Pavement": (0.49, 0.64, 0.61, 1.0),
    "/Game/Maps/Materials/M_Blockout_Interior": (0.075, 0.10, 0.16, 1.0),
    "/Game/Maps/Materials/M_Blockout_Cover": (0.95, 0.14, 0.055, 1.0),
    "/Game/Maps/Materials/M_Blockout_Window": (0.015, 0.48, 0.68, 1.0),
    "/Game/Maps/Materials/M_Blockout_RustedMetal": (0.48, 0.095, 0.045, 1.0),
}


def main():
    updated = []
    for asset_path, color in PALETTE.items():
        material = unreal.load_asset(asset_path)
        if not material:
            unreal.log_warning("Palette material not found: " + asset_path)
            continue

        parameter = None
        for expression in unreal.MaterialEditingLibrary.get_material_expressions(material):
            if isinstance(expression, unreal.MaterialExpressionVectorParameter):
                if expression.get_editor_property("parameter_name") == "BaseColor":
                    parameter = expression
                    break
        if not parameter:
            unreal.log_warning("BaseColor parameter not found: " + asset_path)
            continue

        parameter.set_editor_property("default_value", unreal.LinearColor(*color))
        unreal.MaterialEditingLibrary.recompile_material(material)
        unreal.EditorAssetLibrary.save_loaded_asset(material)
        updated.append(asset_path.rsplit("/", 1)[-1])

    unreal.EditorLevelLibrary.set_level_viewport_camera_info(
        unreal.Vector(8000.0, -9000.0, 7000.0), unreal.Rotator(-32.0, 131.0, 0.0)
    )
    unreal.log("Applied Morrow stylized palette to: " + ", ".join(updated))


main()
