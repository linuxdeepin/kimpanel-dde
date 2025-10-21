# Repository Guidelines

## Project Structure & Module Organization
- Source: `src/` (Qt Widgets/DTK C++).
- Build output: `build/` (created by you, not tracked).
- Build system: `CMakeLists.txt` (Qt6 + Dtk6 + DBus).
- Docs: `dbus.md` for protocol notes, `dtk.md` for DTK/X11 build + testing guidance.
- Legacy references: `kde_example/`, `fcitx5_side_implementation/` provide historical context only.

## Build, Test, and Development Commands
- Configure: `cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug`
- Build: `cmake --build build -j` (produces `build/kimpanel-lite`)
- Run (DDE/X11): `./build/kimpanel-lite`
- Clean build: `rm -rf build && cmake -S . -B build && cmake --build build`

## Coding Style & Naming Conventions
- C++20, Qt style: 4 spaces, no tabs; brace on same line.
- Types/Classes: `PascalCase` (e.g., `KimpanelAdaptor`); methods/vars: `camelCase`; constants/macros: `UPPER_SNAKE_CASE`.
- Files: headers/sources mirror class names (`PanelWindow.h/.cpp`); app entry `main.cpp`.
- Use `clang-format` if available; otherwise follow the above rules consistently.

## Testing Guidelines
- Currently no automated tests. Prefer Qt Test for C++.
- Proposed layout: `tests/` with files named `*_test.cpp`.
- Future run pattern: enable CTest in CMake and run `ctest --output-on-failure` from `build/`.

## Commit & Pull Request Guidelines
- Use Conventional Commits where sensible: `feat:`, `fix:`, `chore:`, `docs:`, `refactor:`.
- Commits should be small and focused; reference issues (`#123`) in the body.
- PRs must include: purpose summary, notable design choices, how to run, and screenshots/GIFs of the panel (note compositor/DE used). Include DBus/log snippets if relevant.
- Update `CMakeLists.txt` when adding/removing sources.

## Security & Configuration Tips
- Runs as a DTK/X11 widget; ensure X11/DDE session is active when testing.
- DBus service path/name remain `org.kde.impanel` at `/org/kde/impanel`; avoid breaking compatibility.
- Dependencies: Qt6 (Core/Gui/Widgets/DBus) and `Dtk6::Widget`, `Dtk6::Gui`, `Dtk6::Core` must be available at build time.

## Agent-Specific Instructions
- Place new C++ in `src/` and adjust `CMakeLists.txt` accordingly.
- Keep changes minimal and scoped; do not alter unrelated files or add licenses.
- Prefer adding options behind safe defaults; don’t regress navigation or DBus contracts.

## Implementation Status: org.kde.kimpanel.inputmethod Signals ✅ COMPLETED

- **KimpanelInputmethodWatcher** subscribes to `UpdateAux`, `ShowAux`, `ShowLookupTable`, `Enable`, `RegisterProperties`, `UpdateProperty`, and `RemoveProperty` signals and updates `KimpanelAdaptor`.
- **KimpanelAdaptor** exposes properties (`auxText`, `auxVisible`, `lookupVisible`, `enabled`) with change detection, caches status properties from Fcitx5, and provides DBus navigation helpers.
- Environment toggle `KIMPANEL_DISABLE_INPUTMETHOD` still disables signal subscription for debugging.

## Implementation Status: Deepin Status Notifier Icon ✅ COMPLETED

- **SystemTrayController** (`src/SystemTrayController.{h,cpp}`) owns a `QSystemTrayIcon`, keeps icon/tooltips in sync with the `/Fcitx/im` property, and exposes a quick switch entry plus quit action in the context menu.
- Left-clicking the tray icon now auto-cycles to the next input method by reusing the `ExecMenu` payload, while the context action still surfaces the full chooser when manual selection is desired.
- **KimpanelAdaptor** emits property change notifications and forwards `TriggerProperty` to `org.kde.impanel` so tray interactions reach Fcitx5.
- Runtime guard `KIMPANEL_DISABLE_SNI` disables the tray path for debugging or environments without SNI; `QSystemTrayIcon::isSystemTrayAvailable()` is checked at startup.

## Implementation Status: DTK/X11 Panel Port ✅ COMPLETED

- **PanelWindow** (`src/PanelWindow.{h,cpp}`) renders auxiliary text and lookup candidates using DTK widgets, uses a compact chip row without navigation buttons (paging stays via keyboard), and tracks Deepin palette changes live so background/text colors follow the active light or dark theme.
- **Application Bootstrap** (`src/main.cpp`) now instantiates `DApplication`, drops LayerShell, and relies on QWidget positioning for caret tracking.
- **Build System** (`CMakeLists.txt`) links against Qt Widgets and Dtk6 components; QML module and LayerShellQt dependencies removed.
- **Documentation** (`dtk.md`, `CLAUDE.md`) updated with DTK dependencies and DDE/X11 manual test steps.

## Future Enhancement Ideas

- Text attribute support for auxiliary content rendering (bold/italic/color spans).
- Service detection via `QDBusServiceWatcher` to handle input method restarts gracefully.
- Extended navigation (candidate selection by index) and property surfaces for language toggles.
- Multi-monitor positioning heuristics and screen bounds clamping for caret moves.
- Optional Wayland back-end reintroduction guarded by a runtime switch once DTK path stabilises.

## Manual Testing Notes

See `dtk.md` for a detailed DDE/X11 checklist covering panel boot, candidate paging, aux text display, dock tray verification, and DBus validation.
