#######################################################
### BibExplorer                                     ###
#######################################################
# Project information
cmake_minimum_required(VERSION 3.14)
project(bibexplorer VERSION 0.0.1)
set(CMAKE_CXX_STANDARD 17)

# CMake dependencies for installer
include(CMakePackageConfigHelpers)
include(GNUInstallDirs)

# Include CPM to find or download packages
# Note that CPM will try to find the packages locally before downloading them
# This avoids ODR problems
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
option(CPM_USE_LOCAL_PACKAGES "Try `find_package` before downloading dependencies" ON)
include(cmake/CPM.cmake)

# Check if this a master project or a subdirectory of another project
if (${CMAKE_CURRENT_SOURCE_DIR} STREQUAL ${CMAKE_SOURCE_DIR})
    set(MASTER_PROJECT ON)
else()
    set(MASTER_PROJECT OFF)
endif()
set(BIBEXPLORER_ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR})

#######################################################
### Options                                         ###
#######################################################
# What to build
option(BUILD_INSTALLER "Build an installation package" ${MASTER_PROJECT})
option(BUILD_PACKAGE "Build an installation package" ${MASTER_PROJECT})
option(BUILD_DEMO_IMGUI "Include options to show ImGui and ImPlot demos on top of the application" OFF)

# How to build
option(BUILD_WITH_PEDANTIC_WARNINGS "Use pedantic warnings. This is useful for developers because many of these warnings will be in continuous integration anyway." ${DEBUG_MODE})
option(BUILD_WITH_UTF8 "Accept utf-8 in MSVC by default." ON)

#######################################################
### Libraries                                       ###
#######################################################
add_subdirectory(source)

#######################################################
### Executable                                      ###
#######################################################
add_subdirectory(standalone)

#######################################################
### Packages                                        ###
#######################################################
if (BUILD_INSTALLER AND BUILD_PACKAGE AND OFF)
    # Set the cpack variables
    # https://cliutils.gitlab.io/modern-cmake/chapters/install/packaging.html

    # The most common cpack variables
    set(CPACK_PACKAGE_NAME "BibExplorer")
    set(CPACK_PACKAGE_VENDOR "https://github.com/alandefreitas")
    set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Bibliometrics Explorer")
    set(CPACK_PACKAGE_VERSION_MAJOR ${PROJECT_VERSION_MAJOR})
    set(CPACK_PACKAGE_VERSION_MINOR ${PROJECT_VERSION_MINOR})
    set(CPACK_PACKAGE_VERSION_PATCH ${PROJECT_VERSION_PATCH})
    set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")
    set(CPACK_RESOURCE_FILE_README "${CMAKE_CURRENT_SOURCE_DIR}/README.md")

    # Set CPACK_SOURCE_IGNORE_FILES with files source packages shouldn't install
    # We get these from .gitignore to avoid redundancy
    FILE(READ .gitignore GITIGNORE_CONTENTS)
    STRING(REGEX REPLACE ";" "\\\\;" GITIGNORE_CONTENTS "${GITIGNORE_CONTENTS}")
    STRING(REGEX REPLACE "\n" ";" GITIGNORE_CONTENTS "${GITIGNORE_CONTENTS}")
    set(CPACK_SOURCE_IGNORE_FILES ${GITIGNORE_CONTENTS})

    # Always include CPack at last
    include(CPack)
endif()