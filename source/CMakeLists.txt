#######################################################
### External libraries                              ###
#######################################################
# Note that CPMAddPackage will try to find_package first
# so there are no ODR violations

# Filesystem
find_package(Filesystem QUIET)
if (NOT CXX_FILESYSTEM_HAVE_FS)
    message(FATAL_ERROR "Your C++ compiler does not have the filesystem library. Please update your C++ compiler.")
endif()

# Threads
find_package(Threads REQUIRED)

# Parse XML: https://github.com/zeux/pugixml
CPMAddPackage(NAME pugixml GITHUB_REPOSITORY zeux/pugixml VERSION 1.10 GIT_TAG v1.10 DOWNLOAD_ONLY TRUE)
if (pugixml_ADDED)
    add_subdirectory(${pugixml_SOURCE_DIR} ${pugixml_SOURCE_DIR}/../pugixml-binary EXCLUDE_FROM_ALL)
else()
    if (NOT TARGET pugixml)
        message(FATAL_ERROR "Cannot find or download pugixml")
    endif()
endif()

# Find curl with CPM to make sure we find it on Windows
find_package(CURL QUIET)
if (NOT CURL_FOUND)
    CPMAddPackage(NAME CURL GITHUB_REPOSITORY curl/curl VERSION 7.72.0 GIT_TAG curl-7_72_0 DOWNLOAD_ONLY TRUE)
    if (CURL_ADDED)
        add_subdirectory(${CURL_SOURCE_DIR} ${CURL_SOURCE_DIR}/../CURL-binary EXCLUDE_FROM_ALL)
    endif()
endif()

# Curlpp: https://github.com/jpbarrette/curlpp
# TODO: Replace with https://whoshuu.github.io/cpr/
CPMAddPackage(NAME curlpp GITHUB_REPOSITORY jpbarrette/curlpp VERSION 0.8.1 GIT_TAG v0.8.1 DOWNLOAD_ONLY TRUE)
if (curlpp_ADDED)
    # Create the library here to avoid calling find_package(CURL REQUIRED) again in the subdirectory
    # We already included CURL with find_package or CPM, but the curlpp build script doesn't know about that
    file(GLOB_RECURSE HeaderFileList "${curlpp_SOURCE_DIR}/include/*")
    file(GLOB_RECURSE SourceFileList "${curlpp_SOURCE_DIR}/src/*")
    add_library(curlpp_static STATIC ${HeaderFileList} ${SourceFileList})
    target_include_directories(curlpp_static PUBLIC ${curlpp_SOURCE_DIR}/include)
    if (CURL_FOUND)
        target_link_libraries(curlpp_static PUBLIC ${CURL_LIBRARIES})
        target_include_directories(curlpp_static PUBLIC ${CURL_INCLUDE_DIRS})
    else()
        target_link_libraries(curlpp_static PUBLIC libcurl)
    endif()
else ()
    if (NOT TARGET curlpp)
        message(FATAL_ERROR "Cannot find or download curlpp")
    endif()
endif()

# Tidy HTML: https://github.com/htacg/tidy-html5
CPMAddPackage(NAME tidy-html5 GITHUB_REPOSITORY htacg/tidy-html5 VERSION 5.7.28 GIT_TAG 5.7.28 DOWNLOAD_ONLY TRUE)
if (tidy-html5_ADDED)
    add_subdirectory(${tidy-html5_SOURCE_DIR} ${tidy-html5_SOURCE_DIR}/../tidy-html5-binary EXCLUDE_FROM_ALL)
    target_include_directories(tidy-static PUBLIC ${tidy-html5_SOURCE_DIR}/include)
else()
    if (NOT TARGET tidy-html5)
        message(FATAL_ERROR "Cannot find or download tidy-html5")
    endif()
endif()

# Serialization: https://github.com/USCiLab/cereal
CPMAddPackage(NAME cereal GITHUB_REPOSITORY USCiLab/cereal VERSION 1.3.0 GIT_TAG v1.3.0 DOWNLOAD_ONLY TRUE)
if (cereal_ADDED)
    add_library(cereal INTERFACE IMPORTED)
    target_include_directories(cereal INTERFACE ${cereal_SOURCE_DIR}/include)
elseif()
    if (NOT TARGET cereal)
        message(FATAL_ERROR "Cannot find or download cereal")
    endif()
endif()

# Logging: https://github.com/gabime/spdlog
CPMAddPackage(NAME spdlog GITHUB_REPOSITORY gabime/spdlog VERSION 1.7.0 GIT_TAG v1.7.0)

# CSV: https://github.com/vincentlaucsb/csv-parser
CPMAddPackage(NAME csv-parser GITHUB_REPOSITORY vincentlaucsb/csv-parser GIT_TAG 7ea4c9b0d76485928764e3339c14d27ed727f14a DOWNLOAD_ONLY TRUE)
if (csv-parser_ADDED)
    add_library(csv INTERFACE IMPORTED)
    target_include_directories(csv INTERFACE ${csv-parser_SOURCE_DIR}/single_include)
else()
    if (NOT TARGET csv-parser AND NOT TARGET csv)
        message(FATAL_ERROR "Cannot find or download csv-parser")
    endif()
endif()

try_compile(SYSTEM_HAS_ISNAN_OUTSIDE_STD ${CMAKE_CURRENT_BINARY_DIR} SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/bibliometrics/common/nantest.cpp)
message("SYSTEM_HAS_ISNAN_OUTSIDE_STD=${SYSTEM_HAS_ISNAN_OUTSIDE_STD}")
if (SYSTEM_HAS_ISNAN_OUTSIDE_STD)
    target_compile_definitions(csv INTERFACE SYSTEM_HAS_ISNAN_OUTSIDE_STD)
endif()

# JSON: https://github.com/nlohmann/json
CPMAddPackage(NAME nlohmann_json VERSION 3.9.1 URL https://github.com/nlohmann/json/releases/download/v3.9.1/include.zip URL_HASH SHA256=6bea5877b1541d353bd77bdfbdb2696333ae5ed8f9e8cc22df657192218cad91)
if (nlohmann_json_ADDED)
    add_library(nlohmann_json INTERFACE IMPORTED)
    target_include_directories(nlohmann_json INTERFACE ${nlohmann_json_SOURCE_DIR}/include)
else()
    if (NOT TARGET nlohmann_json AND NOT TARGET nlohmann_json)
        message(FATAL_ERROR "Cannot find or download nlohmann_json")
    endif()
endif()

# OpenGL
find_package(OpenGL)

# Glad: https://github.com/Dav1dde/glad
find_package(GLAD QUIET)
if (NOT GLAD_FOUND AND NOT TARGET glad)
    # Use CPM only if not found, to avoid ODR violations
    # find_package(GLAD REQUIRE) would suffice if it worked well
    CPMAddPackage(NAME glad GIT_REPOSITORY https://github.com/Dav1dde/glad GIT_TAG df8e9e16110b305479a875399cee13daa0ccadd9 VERSION 0.1.33)
else ()
    # FindGLAD does not usually create a target, so we create an interface target
    if (NOT TARGET glad)
        add_library(glad INTERFACE)
        target_include_directories(glad INTERFACE ${GLAD_INCLUDE_PATH})
        target_link_libraries(glad INTERFACE ${GLAD_LIBRARIES})
    endif ()
endif ()

# GLFW: https://github.com/glfw/glfw
find_package(glfw3 QUIET)
if (NOT GLFW3_FOUND AND NOT TARGET glfw)
    # Use CPM only if not found, to avoid ODR violations
    # find_package(glfw3 REQUIRE) would suffice if it worked well
    CPMAddPackage(NAME glfw3 GIT_REPOSITORY https://github.com/glfw/glfw VERSION 3.3.2 GIT_TAG 3.3.2 OPTIONS "GLFW_BUILD_DOCS OFF" "GLFW_BUILD_EXAMPLES OFF" "GLFW_BUILD_TESTS OFF" "GLFW_INSTALL OFF")
endif ()

# ImGui: https://github.com/ocornut/imgui
CPMAddPackage(NAME imgui GIT_REPOSITORY https://github.com/ocornut/imgui VERSION 1.77 DOWNLOAD_ONLY TRUE)
set(IMGUI_INCLUDE_DIR ${imgui_SOURCE_DIR}/)
file(GLOB IMGUI_SOURCES ${imgui_SOURCE_DIR}/*.cpp)
file(GLOB IMGUI_HEADERS ${imgui_SOURCE_DIR}/*.h)
add_library(imgui STATIC ${IMGUI_SOURCES} ${IMGUI_SOURCES} ${imgui_SOURCE_DIR}/examples/imgui_impl_glfw.cpp ${imgui_SOURCE_DIR}/examples/imgui_impl_opengl3.cpp)
add_definitions(-DIMGUI_IMPL_OPENGL_LOADER_GLAD)
target_include_directories(imgui PUBLIC ${IMGUI_INCLUDE_DIR} ${OPENGL_INCLUDE_DIR} ${GLFW_INCLUDE_DIR} ${GLAD_INCLUDE_DIR})
target_link_libraries(imgui PUBLIC ${OPENGL_LIBRARIES} glfw glad)

# ImPlot: https://github.com/epezent/implot
CPMAddPackage(NAME implot GIT_REPOSITORY https://github.com/epezent/implot GIT_TAG v0.4 VERSION 0.4 DOWNLOAD_ONLY TRUE)
add_library(implot STATIC ${implot_SOURCE_DIR}/implot.h ${implot_SOURCE_DIR}/implot.cpp ${implot_SOURCE_DIR}/implot_demo.cpp)
target_include_directories(implot PUBLIC ${implot_SOURCE_DIR})
target_link_libraries(implot PUBLIC imgui)

# Command line options: https://github.com/jarro2783/cxxopts
CPMAddPackage(NAME cxxopts GIT_REPOSITORY https://github.com/jarro2783/cxxopts GIT_TAG v2.2.0 VERSION 2.2 DOWNLOAD_ONLY TRUE)
if (cxxopts_ADDED)
    add_library(cxxopts INTERFACE IMPORTED)
    target_include_directories(cxxopts INTERFACE ${cxxopts_SOURCE_DIR}/include)
elseif()
    if (NOT TARGET cxxopts)
        message(FATAL_ERROR "Cannot find or download cxxopts")
    endif()
endif()

#######################################################
### Bibliometrics application library               ###
#######################################################
add_library(bibliometrics
        bibliometrics/application/application.h
        bibliometrics/application/application.cpp
        bibliometrics/common/system.h
        bibliometrics/common/algorithm.h
        bibliometrics/common/algorithm.cpp
        bibliometrics/graph/paper.h
        bibliometrics/graph/paper.cpp
        bibliometrics/graph/author.h
        bibliometrics/graph/author.cpp
        bibliometrics/graph/author_group.h
        bibliometrics/graph/author_group.cpp
)

# PRIVATE dependencies (the ones we use in .cpp files)
target_link_libraries(bibliometrics PRIVATE ${CMAKE_THREAD_LIBS_INIT} imgui implot csv)

# PUBLIC dependencies (the ones we use in .h files)
target_link_libraries(bibliometrics PUBLIC std::filesystem glad glfw ${CMAKE_DL_LIBS} nlohmann_json spdlog curlpp_static pugixml cereal cxxopts tidy-static)

# Include directory
target_include_directories(bibliometrics
        PUBLIC $<BUILD_INTERFACE:${BIBEXPLORER_ROOT_DIR}/source>
               $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)

# Definition to find the proper filesystem library
if (CXX_FILESYSTEM_IS_EXPERIMENTAL)
    target_compile_definitions(bibliometrics PUBLIC CXX_FILESYSTEM_IS_EXPERIMENTAL)
endif()

# Definition to calcel the ImGui demos. This is useful for development
if (BUILD_DEMO_IMGUI)
    target_compile_definitions(bibliometrics PUBLIC BUILD_DEMO_IMGUI)
endif()

# Allow utf-8 on windows
if (BUILD_WITH_UTF8 AND MSVC)
    target_compile_options(bibliometrics PRIVATE /utf-8)
endif()

# Hack to check for min in Windows.h and cancel it
# http://www.suodenjoki.dk/us/archive/2010/min-max.htm
include(CheckSymbolExists)
check_symbol_exists(min "Windows.h" HAVE_WINDOWS_MINMAX)
if (HAVE_WINDOWS_MINMAX)
    target_compile_definitions(bibliometrics PUBLIC NOMINMAX)
endif()

# Set warning levels to about the same level for MSVC, GCC, and Clang
if (BUILD_WITH_PEDANTIC_WARNINGS)
    if (MSVC)
        target_compile_options(bibliometrics PRIVATE /W4 /WX)
    else ()
        target_compile_options(bibliometrics PRIVATE -Wall -Wextra -pedantic -Werror)
    endif ()
endif ()

#######################################################
### Installer                                       ###
#######################################################
# The installer does not include the bibliometrics
# library because it does not make much sense.
# The library is not meant to be used directly because
# most relevant processing happens in application.cpp.
# Other classes are just structures to keep information
# about the papers and authors.