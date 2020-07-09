cmake_minimum_required(VERSION 3.10 FATAL_ERROR)

project(Katty)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt5 REQUIRED
COMPONENTS
    Core
    Gui
    Sql
    WebSockets
    Widgets
)

add_subdirectory("Shared")
add_subdirectory("Client")
add_subdirectory("Server")
