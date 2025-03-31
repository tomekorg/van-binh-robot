cmake_minimum_required(VERSION 3.5)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(tomek)

add_custom_target(prebuild
    COMMAND ${CMAKE_COMMAND} -E chdir ${CMAKE_CURRENT_SOURCE_DIR} ./prebuild-web-ui.sh
    COMMENT "Prebuild: Building web-ui to serve from http"
)

add_dependencies(${PROJECT_NAME}.elf prebuild)
