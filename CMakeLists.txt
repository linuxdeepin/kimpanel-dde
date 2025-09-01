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

find_package(Qt6 REQUIRED COMPONENTS Core Gui Qml Quick DBus)
find_package(LayerShellQt REQUIRED)

qt_add_executable(kimpanel-lite
  src/main.cpp
  src/KimpanelAdaptor.cpp
  src/KimpanelAdaptor.h
)
target_link_libraries(kimpanel-lite PRIVATE Qt6::Core Qt6::Gui Qt6::Qml Qt6::Quick Qt6::DBus LayerShellQt::Interface)

qt_add_qml_module(kimpanel-lite
  URI KimpanelLite
  VERSION 1.0
  QML_FILES
    qml/Main.qml
)
install(TARGETS kimpanel-lite)
