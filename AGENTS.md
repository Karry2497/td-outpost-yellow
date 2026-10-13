# Project guidance

- Use Unreal Engine 5.8 and keep the existing C++ project/module name `extractionFPS` unless a deliberate rename is requested.
- Use the Unreal Editor MCP for editor state and asset changes when its server is available. Its calls run on the game thread; make MCP calls sequentially.
- Prefer small, reviewable changes. For C++ changes, report the build or compile result; do not claim success without evidence.
- Keep generated Unreal directories and local editor state out of Git. Track `.uasset` and `.umap` files through Git LFS.
- After each user-approved iteration, review the diff, verify it in the editor where relevant, record reusable lessons in `docs/AI/LEARNING_LOG.md`, commit the finished slice, and let the configured post-commit hook push it.
- Treat one-off failures as observations. Add a lasting lesson only when it is actionable and likely to recur.
