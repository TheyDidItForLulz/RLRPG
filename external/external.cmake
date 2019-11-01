message(STATUS "Getting submodules")

message(STATUS "Looking for yaml-cpp")
find_package(yaml-cpp QUIET)

if (NOT yaml-cpp_FOUND)
    message(STATUS "yaml-cpp not found, building from sources")
    execute_process(COMMAND git submodule update --init --recursive -- external/yaml-cpp
                    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
    add_subdirectory(external/yaml-cpp)

    set(RLRPG_YAML_TARGET yaml::yaml)
else()
    set(RLRPG_YAML_TARGET ${YAML_CPP_LIBRARIES})
endif()

message(STATUS "Looking for fmtlib")
find_package(fmt QUIET)

if (NOT fmt_FOUND)
    message(STATUS "fmt not found, building from sources")
    execute_process(COMMAND git submodule update --init --recursive -- external/fmt
                    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
    add_subdirectory(external/fmt)
endif()

