# AI Straight-line Nerf (EXPERIMENTAL)

> # ⚠ EXPERIMENTAL ⚠
> **This addon is experimental and barely tested. It may do nothing, behave differently on your setup, or break with a game or launcher update. Use it at your own risk and do not rely on it.**

**EXPERIMENTAL, no promises.** For F1 25 on Steam (build 25251167 or newer) with **Swofty Launcher 1.8.19 or newer**. Tested in Grand Prix races only.

> **Please read first**
> - **EXPERIMENTAL.** We tested that the settings load and that the AI cars get the power cut (the log shows `[ai-straight-nerf] 23 AI car(s): power -2%`). How much it changes the gaps on the straights is not measured.
> - **The ERS option is a game database edit.** It needs a **game restart** and only covers **the game's own cars** (the 2025/2026 cars, classics, older seasons, F2). Cars added by other mods keep the game's ERS values. We also do not know how much the AI's battery really matters in the game's own AI.
> - **The power and top-speed cuts are live and work on every AI car, modded cars included.** No restart, no new race needed: they apply within a second.
> - **A cut also slows the AI on corner exits.** Corner speed itself is not touched (the AI plans it from grip, not power).

## What is it?
AI cars in F1 25 are quick on the straights and never seem to run out of ERS. This addon slows them there without making them slower in the corners.

## Settings
- **AI ERS: stops deploying at battery.** The battery level where the AI stops using ERS: stock 10%, or 25% / 40% so its battery runs dry sooner. Restart needed, the game's own cars only.
- **AI engine power cut (%).** 0 to 10. Takes power from every AI car. Live.
- **AI top speed cut (rpm).** 0 to 600. The AI engine torque is cut this many rpm under the limiter, which caps the speed at the end of long straights. Live.
- **Open the panel when the game is paused.** Off by default.

## Mouse panel
Press **F8** in a session to open a panel with two sliders (power cut and top speed cut) and a Reset button. Drag them with the mouse. Esc, the X or F8 closes it. While it is open the panel holds the mouse, so the game does not get your keys: close it before you unpause. Values set in the panel are not saved; the launcher's settings load again at the next start. Needs Swofty's overlay.

## Your car
The cuts apply to a car while the AI drives it (for example with AI Drive), and are released when you take it back.
