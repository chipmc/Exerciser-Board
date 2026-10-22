# WO-001: Initialize the git repo and first commit

**Author:** Claude (architect, in the app)
**Executor:** Claude Code (workflow controller), Sonnet-class model is sufficient; this is mechanical.
**Governing process:** Chip's multi-agent workflow (AI_DEVELOPMENT_WORKFLOW.md in the Generalized-Core-Counter repo; copying it into this repo is a separate decision for Chip).
**Size budget:** 0 firmware (`src/`) lines changed. No new code. Only repo setup files.

## Goal (plain language)
Put this project under git so we can track the bring-up guide, the PlatformIO config and the firmware, and so future work orders have a home.

## Context
- Folder: the Exerciser-Board PlatformIO project (ATtiny3226 exerciser board).
- `docs/bring-up-guide.md` is the current bring-up guide; `docs/work-orders/` holds work orders.
- `platformio.ini` and `src/main.cpp` were set up by hand and have NOT been built or uploaded yet. Do not try to build or upload; Chip does all device operations.

## Steps
1. Run `git status`. If this folder is already a git repo, stop and report; do not re-init.
2. `git init -b main`.
3. Read the existing `.gitignore` (PlatformIO generated). Confirm it excludes `.pio/` and the generated `.vscode/` files. Add only what is missing (for example `.DS_Store`). Do not add anything else.
4. Add a short `README.md` (10 lines or fewer): what the exerciser board is, that the bring-up guide is in `docs/bring-up-guide.md`, and that work orders live in `docs/work-orders/`.
5. Stage everything not ignored and make one commit: "Initial exerciser project: PlatformIO config, blinky, bring-up guide".
6. Do NOT create a remote or push. Chip will add the GitHub remote.

## Done when
- `git log` shows one commit and `git status` is clean.
- `.pio/` is not tracked.
- Report back: the commit hash, the list of tracked files, and any `.gitignore` additions.

## Closing record (Claude Code fills in)
Budget vs actual (lines, `src/` vs other):
Notes:
