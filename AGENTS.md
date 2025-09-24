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

## Implementation Status: org.kde.kimpanel.inputmethod Signals ✅ COMPLETED

The `org.kde.kimpanel.inputmethod` D-Bus interface support has been successfully implemented:

### Completed Components
- **KimpanelInputmethodWatcher** (`src/KimpanelInputmethodWatcher.{h,cpp}`)
  - Subscribes to `UpdateAux(QString,QString)`, `ShowAux(bool)`, `ShowLookupTable(bool)`, `Enable(bool)` signals
  - Environment toggle: `KIMPANEL_DISABLE_INPUTMETHOD` to disable subscription for debugging
  - Updates KimpanelAdaptor state via setter methods

- **Extended KimpanelAdaptor** (`src/KimpanelAdaptor.{h,cpp}`)
  - Added Q_PROPERTY: `auxText`, `auxVisible`, `lookupVisible`, `enabled` 
  - Setter methods with change detection to avoid unnecessary signal emissions
  - New signals: `auxChanged`, `lookupVisibleChanged`, `enabledChanged`

- **QML Integration** (`qml/Main.qml`)
  - Panel visibility: `(PanelAdaptor.lookupVisible || PanelAdaptor.auxVisible)`
  - Auxiliary text display when `auxVisible` is true
  - Dynamic height calculation including aux text area

- **Build System** (`CMakeLists.txt`)
  - Added `KimpanelInputmethodWatcher.{cpp,h}` to sources

- **Documentation** (`dbus.md`)
  - Updated with inputmethod signal descriptions and QML property mappings

### Current Capabilities
- Receives and displays auxiliary text from input methods (e.g., Fcitx5 pronunciation hints)
- Controls lookup table and auxiliary text visibility independently
- Tracks input method enabled state (available as QML property)
- Maintains backward compatibility with existing `org.kde.impanel2` functionality
- Environment-based toggle for debugging/testing

### Testing
- Manual testing with Fcitx5: `QT_QPA_PLATFORM=wayland ./build/kimpanel-lite`
- Use `qdbus` or `busctl` to emit test signals to verify behavior
- No regressions to existing `impanel2` spot positioning or lookup table functionality

## Future Enhancement Ideas

### Text Attributes Support
- Parse and apply text attributes from `UpdateAux` signal's second parameter
- Support for text formatting (bold, italic, colors) in auxiliary text display

### Service Detection
- Add `QDBusServiceWatcher` to detect when `org.kde.kimpanel.inputmethod` service appears/disappears
- Dynamic subscription management based on service availability

### Input Method Properties
- Implement `RegisterProperties` and `UpdateProperty` signal handling
- Add property-based configuration UI (language switching, input mode toggles)

### Performance Optimizations  
- Batch multiple signal updates to reduce QML redraws
- Implement smart visibility management to avoid unnecessary panel shows/hides

### Enhanced Positioning
- Support for multiple monitor setups with proper panel positioning
- Adaptive positioning when panel would appear off-screen

## Next Steps: DTK/X11 Panel Port

1. Audit DTK building blocks for an X11 panel window (dtkwidget/dtkgui) and capture required components for Deepin styling.
2. Extract the existing panel logic into DTK-ready classes and remove Wayland/LayerShell dependencies so the application boots natively on DDE/X11.
3. Prototype a DTK-based panel surface (DApplication + frameless DWidget) that renders the existing state and fixes the DDE/X11 glitches.
4. Rework candidate rendering for the DTK panel so candidates lay out in a horizontal row with proper spacing, highlighting, and navigation hooks.
5. Delete the old Wayland/QML panel path from the build, wiring all entry points and resources to the new DTK6 implementation.
6. Update build docs with DTK dependencies and draft manual test steps on DDE/X11 (panel rendering, candidate row behavior, DBus updates).

(Tray icon integration will be revisited after the DTK/X11 panel foundation is stable.)

### DTK/X11 Panel Stack Decision
- Target DTK6 with DtkCore + DtkGui + DtkWidget; they ship together and give Deepin-native theming, window helpers, and widget primitives.
- Build the panel as a `DApplication` + frameless `DWidget`; use DTK layouts (`QHBoxLayout`/`DLabel` etc.) to render candidates horizontally and apply palette-aware styling.
- Keep existing DBus/state logic toolkit-agnostic and drive the widget view via signal/slot updates.
- Only pull in DtkDeclarative later if we intentionally reintroduce a QML surface; the initial X11 port stays purely widget-based.
- Remove the legacy Wayland/QML stack and rely solely on the DTK6/DDE/X11 implementation moving forward.
