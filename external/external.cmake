message(STATUS "Getting submodules")

message(STATUS "Looking for fmtlib")
find_package(fmt QUIET)

if (NOT fmt_FOUND)
    execute_process(COMMAND git submodule update --init --recursive -- external/fmt
                    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
    add_subdirectory(external/fmt)
endif()

