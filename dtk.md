# DTK/X11 Panel Notes

## Build Prerequisites
- Qt 6 (Core, Gui, Widgets, DBus)
- Dtk6 components: `dtkcore`, `dtkgui`, `dtkwidget`
- X11 session (Deepin Desktop Environment) for native DTK rendering
- Session DBus available

## Configure & Build
```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build -j
```

## Running
```bash
./build/kimpanel-lite
```

## Manual Test Checklist (DDE/X11)
1. **Panel Boot** – Launch `kimpanel-lite` and confirm a frameless DTK window appears only after candidates arrive.
2. **Candidate Layout** – Trigger candidate updates (e.g., via fcitx5) and ensure entries render in a horizontal row with label/text/comment styling.
3. **Selection Highlight** – Move the input method cursor and confirm the active candidate chip highlights using the DTK palette.
4. **Navigation Buttons** – Click the ◀/▶ buttons; verify the input method receives `LookupTablePageUp/Down` DBus calls and pages candidates accordingly.
5. **Auxiliary Text** – Emit `UpdateAux` + `ShowAux(true)` and ensure the aux banner renders above the candidate row.
6. **Spot Positioning** – Confirm the window moves near the caret when `SetSpotRect` is invoked.
7. **Visibility Rules** – Check the panel hides when lookup and aux data are both hidden or when the input method disables itself.

Collect `journalctl --user -u fcitx5` or `busctl monitor org.kde.kimpanel.inputmethod` logs if any DBus transitions misbehave.
