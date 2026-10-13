# Extraction FPS

Unreal Engine 5.8 C++ first-person project. The starter project currently uses the Unreal First Person template; the game design and extraction loop will be defined in subsequent setup steps.

## Open the project

Open [`extractionFPS.uproject`](extractionFPS.uproject) with Unreal Engine 5.8. The project uses the C++ First Person template and Epic's experimental Unreal MCP plugins (`ModelContextProtocol` and `AllToolsets`).

The Unreal MCP server runs inside the editor on `http://127.0.0.1:8000/mcp`. Keep the editor open while using MCP. It is configured in Codex as `unreal-mcp`.

## Repository setup

- Unreal binary assets use Git LFS. Git LFS must be installed before cloning or pushing asset changes.
- `Binaries`, `DerivedDataCache`, `Intermediate`, `Saved`, and local IDE data are ignored.
- Commits automatically push to the current branch when a remote is reachable. The local Git hook handles the push after each commit; use an explicit commit when an iteration is ready to share.

## AI iteration loop

Each gameplay or tooling iteration records its goal, acceptance criteria, implementation, in-editor verification, and learning in [`docs/AI/ITERATION_LOOP.md`](docs/AI/ITERATION_LOOP.md). Durable project decisions live in [`docs/AI/PROJECT_MEMORY.md`](docs/AI/PROJECT_MEMORY.md), and reusable lessons are summarized in [`docs/AI/LEARNING_LOG.md`](docs/AI/LEARNING_LOG.md).
