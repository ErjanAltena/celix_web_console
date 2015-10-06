
cmake_minimum_required(VERSION 2.8)

project(web_console C)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "/opt/local/share/celix/cmake/modules")
set(CMAKE_C_FLAGS " -g -D_GNU_SOURCE -std=gnu99 -Wall ${CMAKE_C_FLAGS}")

find_package(CELIX REQUIRED)
include_directories(${CELIX_INCLUDE_DIRS})

find_package(PkgConfig REQUIRED)

add_subdirectory(web_console)
add_subdirectory(web_console_bundle_service)
add_subdirectory(web_console_services_service)

deploy_targets()
