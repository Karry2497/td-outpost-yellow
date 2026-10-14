# Reusable learning

Add a short entry only when an iteration produces a specific, repeatable lesson that can improve later work. Link the commit or iteration when useful. Do not store credentials, personal data, or speculative conclusions here.

## Lessons

- In Unreal Engine 5.8, run project Python builders with `UnrealEditor-Cmd.exe <project.uproject> -run=pythonscript -script=<script.py>`. Use `LevelEditorSubsystem` for creating, loading and saving maps; `EditorLevelLibrary` emits deprecation warnings.
- A Python builder that regenerates the active map should clear its actors in place. `NewLevel` rejects a map package that already exists, even after an editor asset deletion attempt.
