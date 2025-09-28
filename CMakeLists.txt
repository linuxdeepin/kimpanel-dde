cmake_minimum_required(VERSION 3.20)
project(kimpanel-lite LANGUAGES CXX)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Debug configuration
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()
set(CMAKE_CXX_FLAGS_DEBUG "-g -O0 -DDEBUG")

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets DBus)
find_package(Dtk6 REQUIRED COMPONENTS Widget Gui Core)

qt_add_executable(kimpanel-lite
  src/main.cpp
  src/KimpanelAdaptor.cpp
  src/KimpanelAdaptor.h
  src/KimpanelInputmethodWatcher.cpp
  src/KimpanelInputmethodWatcher.h
  src/SystemTrayController.cpp
  src/SystemTrayController.h
  src/PanelWindow.cpp
  src/PanelWindow.h
)
target_link_libraries(kimpanel-lite PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::DBus
    Dtk6::Widget
    Dtk6::Gui
    Dtk6::Core)
install(TARGETS kimpanel-lite)
