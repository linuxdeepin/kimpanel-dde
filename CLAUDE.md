# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

This repository implements a **Kimpanel-compatible input method panel** for **Fcitx5** on **Wayland (Hyprland)** using **Qt 6**. It provides a modern, clean candidate selection interface that follows the Fcitx5 Kimpanel DBus protocol.

**Key Components:**
- **main.cpp**: Application entry point with DBus setup and LayerShell window management
- **KimpanelAdaptor**: DBus protocol handler that manages lookup tables and caret positioning
- **Main.qml**: Modern QML UI with dynamic sizing and smooth animations

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

**Data Flow:** Input Method → (DBus) → KimpanelAdaptor → (Qt Signals) → QML UI

**DBus Integration:**
- Registers services: `org.kde.impanel` and `org.kde.impanel2`
- Object path: `/org/kde/impanel` 
- Implements `SetLookupTable()` and `SetSpotRect()` protocol methods
- Emits `PanelCreated2` signal on startup

**Wayland Layer-Shell:**
- Uses LayerShellQt for proper Wayland surface handling
- Positions panel via margin-based coordinate system
- Anchored to TOP+LEFT, LayerTop level, no exclusive zone

## Development Standards

**C++ Style:**
- C++20 with Qt 6 conventions
- No raw `new` - use RAII patterns
- Validate all DBus input indices and handle gracefully
- Log DBus calls with interface, member, and signature info

**QML Guidelines:**  
- Keep delegates lightweight
- Prefer Models/Signals over heavy JavaScript bindings
- Use `pragma ComponentBehavior: Bound` for type safety

**Protocol Compliance:**
- Never crash on unexpected payloads
- Continue gracefully if `org.kde.impanel` service name is taken
- Truncate overly long lists in logs for safety

## Dependencies

- Qt 6 (Core, Gui, DBus, Qml, Quick)
- LayerShellQt library  
- Running Wayland session (preferably Hyprland)
- Session DBus availability

## Key Files

- `src/main.cpp` - Application setup, DBus registration, positioning logic
- `src/KimpanelAdaptor.h/.cpp` - DBus protocol implementation  
- `qml/Main.qml` - Modern panel UI with candidate display
- `CMakeLists.txt` - Qt 6 build configuration with QML module