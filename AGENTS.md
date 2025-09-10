# Repository Guidelines

## Project Structure & Module Organization
- Source: `src/` (C++), QML UI: `qml/`.
- Build output: `build/` (created by you, not tracked).
- Build system: `CMakeLists.txt` (Qt6 + LayerShellQt + DBus).
- Examples/notes: `kde_example/`, `fcitx5_side_implementation/`, docs like `dbus.md`.

## Build, Test, and Development Commands
- Configure: `cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug`
- Build: `cmake --build build -j` (produces `build/kimpanel-lite`)
- Run (Wayland): `QT_QPA_PLATFORM=wayland ./build/kimpanel-lite`
- Clean build: `rm -rf build && cmake -S . -B build && cmake --build build`

## Coding Style & Naming Conventions
- C++20, Qt style: 4 spaces, no tabs; brace on same line.
- Types/Classes: `PascalCase` (e.g., `KimpanelAdaptor`); methods/vars: `camelCase`;
  constants/macros: `UPPER_SNAKE_CASE`.
- Files: headers/sources mirror class names (`KimpanelAdaptor.h/.cpp`); app entry `main.cpp`.
- QML files use `PascalCase.qml` with concise IDs (`main`, `listView`).
- Use `clang-format` if available; otherwise follow the above rules consistently.

## Testing Guidelines
- Currently no automated tests. Prefer Qt Test for C++ and QML test utilities.
- Proposed layout: `tests/` with files named `*_test.cpp` and QML spec files in `tests/qml/`.
- Future run pattern: enable CTest in CMake and run `ctest --output-on-failure` from `build/`.

## Commit & Pull Request Guidelines
- Use Conventional Commits where sensible: `feat:`, `fix:`, `chore:`, `docs:`, `refactor:`.
- Commits should be small and focused; reference issues (`#123`) in the body.
- PRs must include: purpose summary, notable design choices, how to run, and screenshots/GIFs of the panel (note compositor/DE used). Include DBus/log snippets if relevant.
- Update `CMakeLists.txt` when adding/removing sources or QML modules.

## Security & Configuration Tips
- Wayland: the app sets `QT_QPA_PLATFORM=wayland` by default; keep for Hyprland/Sway.
- DBus: service path/name are `org.kde.impanel` at `/org/kde/impanel`; avoid breaking compatibility.
- Dependencies: Qt6 (Core/Gui/Qml/Quick/DBus) and `LayerShellQt::Interface` must be available at build time.

## Agent-Specific Instructions
- Place new C++ in `src/`, QML in `qml/`, and adjust `CMakeLists.txt` accordingly.
- Keep changes minimal and scoped; do not alter unrelated files or add licenses.
- Prefer adding options behind safe defaults; don’t regress Wayland behavior or DBus contracts.

## Next Steps Plan: Add org.kde.kimpanel.inputmethod Signals
- Confirm signal scope and signatures
  - Verify exact signatures for `UpdateAux(ss)`, `ShowAux(b)`, `ShowLookupTable(b)`, `Enable(b)` using `fcitx5_side_implementation/` and `kde_example/`. Defer uncertain details for manual check.

- Add DBus listener class (inputmethod)
  - Create `src/KimpanelInputmethodWatcher.{h,cpp}` subscribing via `QDBusConnection::connect` to `org.kde.kimpanel.inputmethod` signals. No changes to existing `org.kde.impanel2` handling.

- Store aux/lookup visibility + text
  - Extend state holder (expose via `KimpanelAdaptor` or a new context object) with: `auxText` (QString), `auxVisible` (bool), `lookupVisible` (bool). Ignore text attributes initially.

- Track Enable state and expose to QML
  - Maintain `enabled` bool from `Enable(b)` and expose as `Q_PROPERTY` for QML indicator/behavior.

- Wire updates into existing UI
  - In QML, bind visibility to `auxVisible`/`lookupVisible`; display `auxText`. Do not change positioning or existing `impanel2` lookup-table flow.

- Gate via runtime detection/flag
  - Subscribe only when the service appears (optional `QDBusServiceWatcher`). Optionally add a runtime switch (env/CLI) to disable inputmethod path for debugging.

- Update CMake and docs
  - Add new sources to `CMakeLists.txt`. Document added properties/signals usage in `dbus.md` briefly.

- Manual test with Fcitx5/qdbus
  - Run Wayland: `QT_QPA_PLATFORM=wayland ./build/kimpanel-lite`. Validate with Fcitx5 and `qdbus`/`busctl` by emitting `UpdateAux`, `ShowAux`, `ShowLookupTable`, `Enable` and observe UI/state. Ensure no regressions to `impanel2` (spot/lookup).
