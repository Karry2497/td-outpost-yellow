# AI-assisted iteration loop

Use this loop for each gameplay, tools, or project-setup change.

## 1. Define the iteration

Record the player's or developer's requested outcome and observable acceptance criteria. Keep one iteration small enough to review and verify independently.

## 2. Inspect before changing

Read the relevant C++/Blueprint assets, current project memory, recent learning notes, and editor state. Ask the Unreal MCP for live editor facts when it is connected. Do not infer that an asset or build state exists when it has not been inspected.

## 3. Implement the smallest useful slice

Make the change in the project's existing conventions. Keep content changes in the Unreal project and source/control guidance in text files. Avoid unrelated cleanup.

## 4. Verify in Unreal

Use the editor/MCP to inspect the affected asset or behavior. For C++ changes, build the editor target before claiming the change works. For gameplay changes, run the relevant Play In Editor scenario and compare the observed result with the acceptance criteria. Record anything that could not be checked.

## 5. Review and learn

Review the diff and summarize what changed, why, the verification result, and any limitations. Update `PROJECT_MEMORY.md` for durable decisions. Add only actionable, repeatable lessons to `LEARNING_LOG.md`; do not turn a single unexplained failure into a general rule.

## 6. Commit and push

When the iteration is complete, create a focused commit with a clear message. The project post-commit hook pushes the current branch automatically. If the push fails, keep the commit, report the error, and retry after resolving connectivity or authentication.

## Iteration record

Use [`ITERATION_TEMPLATE.md`](ITERATION_TEMPLATE.md) for substantial iterations. Keep routine changes in the commit history rather than adding a diary entry for every edit.
