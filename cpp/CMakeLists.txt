cmake_minimum_required(VERSION 3.5.2)
project(moderntitlebar)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(GODOTCPP_TARGET "$<$<CONFIG:Debug>:template_debug>$<$<NOT:$<CONFIG:Debug>>:template_release>"
    CACHE STRING "" FORCE)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/external/godot-cpp)

file(GLOB_RECURSE MY_SRC CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.h"
)

get_target_property(NAME_SUFFIX godot-cpp GODOTCPP_SUFFIX)

add_library(moderntitlebar SHARED ${MY_SRC})
target_link_libraries(moderntitlebar PRIVATE godot::cpp)
target_link_libraries(moderntitlebar PRIVATE dwmapi)
target_include_directories(moderntitlebar PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)

set_target_properties(moderntitlebar PROPERTIES
    OUTPUT_NAME moderntitlebar${NAME_SUFFIX}
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../addons/moderntitlebar/
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../addons/moderntitlebar/
    ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../addons/moderntitlebar/
)
