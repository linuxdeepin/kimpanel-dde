# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

This repository implements a **Kimpanel-compatible input method panel** for **Fcitx5** on **Deepin/DDE (X11)** using **Qt 6** with the **DTK widget stack**. It exposes the `org.kde.impanel2` DBus surface and renders a frameless DTK window that tracks caret position and candidate state.

**Key Components:**
- **main.cpp**: Application entry point with DBus setup and DTK application bootstrap
- **KimpanelAdaptor**: DBus protocol handler that manages lookup tables, caret positioning, and navigation helpers
- **PanelWindow**: DTK widget hierarchy that renders auxiliary text, candidate chips, and navigation buttons

## Build Commands

```bash
# Standard CMake build
mkdir -p build && cd build
cmake ..
make

# Or use cmake build shortcut
cmake --build build

# Clean build
rm -rf build
```

## Run Commands

```bash
# From build directory
./kimpanel-lite

# From project root
./build/kimpanel-lite
```

## Architecture

**Data Flow:** Input Method → (DBus) → KimpanelAdaptor → (Qt Signals) → PanelWindow (DTK widgets)

**DBus Integration:**
- Registers services: `org.kde.impanel` and `org.kde.impanel2`
- Object path: `/org/kde/impanel`
- Implements `SetLookupTable()` and `SetSpotRect()` protocol methods
- Emits navigation requests back to the input method via `LookupTablePageUp/Down`

**DTK Windowing:**
- Uses `DApplication` + `DWidget` for a frameless, translucent panel window
- Positions panel directly with `QWidget::move()` when caret rectangles update
- Candidate list rendered as horizontal chips with DTK palette-aware styling

## Development Standards

**C++ Style:**
- C++20 with Qt Widgets conventions
- Prefer RAII ownership for Qt objects where practical
- Validate all DBus payloads and guard against empty lookup data
- Keep logging concise but informative for DBus traffic and positioning actions

**Widget Guidelines:**
- Reuse DTK components (`DLabel`, `DPushButton`, `DFrame`) to benefit from theme integration
- Avoid custom painting unless DTK styling cannot cover the requirement
- Keep per-candidate widgets lightweight to minimize redraw overhead

**Protocol Compliance:**
- Never crash on unexpected payloads
- Continue gracefully if `org.kde.impanel` service name is taken
- Confirm navigation requests only fire when input method advertises prev/next pages

## Dependencies

- Qt 6 (Core, Gui, Widgets, DBus)
- Dtk6 (Core, Gui, Widget)
- X11 session (Deepin Desktop Environment) with session DBus

## Key Files

- `src/main.cpp` – Application setup, DBus registration, DTK bootstrap
- `src/KimpanelAdaptor.h/.cpp` – DBus protocol implementation and navigation hooks
- `src/PanelWindow.h/.cpp` – DTK widget tree for panel rendering
- `CMakeLists.txt` – Qt Widgets + DTK build configuration
- `dtk.md` – DTK dependency notes and manual X11 test checklist
