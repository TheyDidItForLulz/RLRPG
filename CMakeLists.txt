cmake_minimum_required(VERSION 3.12)
project(RLRPG)

set(CMAKE_CXX_STANDARD 17)

include_directories(include)
include_directories(include/termlib)

find_package(fmt REQUIRED)
find_package(Curses REQUIRED)

add_executable(RLRPG
        include/termlib/abstract_terminal_window.hpp
        include/termlib/default_window_provider.hpp
        include/termlib/ncurses_color_pair.hpp
        include/termlib/ncurses_terminal_window.hpp
        include/termlib/terminal_color.hpp
        include/termlib/terminal_reader.hpp
        include/termlib/terminal_renderer.hpp
        include/termlib/terminal_text_style.hpp
        include/termlib/termlib.hpp
        include/termlib/vec2.hpp
        include/array2d.hpp
        include/controls.hpp
        include/direction.hpp
        include/enemy.hpp
        include/gen_map.hpp
        include/globals.hpp
        include/hero.hpp
        include/inventory.hpp
        include/inventory_iterator.hpp
        include/item.hpp
        include/level.hpp
        include/log.hpp
        include/unit.hpp
        include/utils.hpp
        src/termlib/default_window_provider.cpp
        src/enemy.cpp
        src/gen_map.cpp
        src/hero.cpp
        src/inventory.cpp
        src/item.cpp
        src/level.cpp
        src/log.cpp
        src/main.cpp
        README.md
        tips.txt
        src/unit.cpp
        src/utils.cpp)

target_link_libraries(RLRPG fmt::fmt ${CURSES_LIBRARIES})
