cmake_minimum_required(VERSION 3.5)
project(v4l2dev VERSION 1.0.0 LANGUAGES CXX)

set(DEFAULT_BUILD_TYPE "Release")

include(GNUInstallDirs)

file(GLOB SOURCE_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/v4l2dev.cpp )
file(GLOB SOURCE_HEADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/v4l2dev.h
    )

add_library(${PROJECT_NAME} SHARED
		${SOURCE_FILES}
            ${SOURCE_HEADERS})

target_include_directories(${PROJECT_NAME} PRIVATE include )
target_include_directories(${PROJECT_NAME} PRIVATE src)


target_link_libraries (${PROJECT_NAME} udev)
set_target_properties(${PROJECT_NAME} PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION 1)
