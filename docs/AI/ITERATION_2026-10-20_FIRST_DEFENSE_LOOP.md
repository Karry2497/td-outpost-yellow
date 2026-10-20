# Iteration: First outpost-defense loop

- **Goal:** Turn the existing third-person Morrow District into a small playable tower-defense prototype while retaining the shooter weapon.
- **Acceptance criteria:** Spawn yellow enemies in waves on the three road approaches; reward kills; let the player spend credits on turret, guard bot, barbed wire, wall repair/reinforcement, ammo, and an airstrike; keep the existing shooter Blueprint as the configured game mode.
- **Change:** Added native zombie archetypes and wave management, basic colored deployable defenses, purchase actions on number keys 1–7, wall integrity upgrades, ammo refill integration, and project setup documentation.
- **Verification:** UE 5.8 `extractionFPSEditor` build succeeded. In Play In Editor the third-person character and shooter HUD appeared, the first wave announced six incoming zombies, and pressing `1` visibly spawned a sentry. Full enemy travel/combat and hotkeys 2–7 are still unverified. The editor viewport turned black after stopping PIE, so its post-play level view needs recovery. Unreal MCP itself was not used for this verification.
- **Outcome:** The compiled first gameplay slice is working in PIE at a basic level. This remains a prototype; the polished HUD, balance, model art and full wave playtest remain open.
- **Reusable lesson:** Keep the existing Shooter Blueprint in the inheritance chain so its weapon, character and HUD assets remain active while native code adds the wave-defense rules.
- **Follow-up:** Recover the editor level viewport, run a full wave to verify zombie movement and attacks at all three walls, try hotkeys 2–7, then apply and review the optional stylized palette script. Improve deployable placement, HUD and balance after that playtest.
