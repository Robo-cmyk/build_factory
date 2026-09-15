
cmake_minimum_required(VERSION 3.31.6)

project(build_factory
        VERSION 0.1.0
        LANGUAGES CXX)

set(RAYLIB_VERSION 6.0)
set(NLOHMANN_JSON_VERSION v3.12.0)

option(BUILD_FACTORY_USE_SYSTEM_RAYLIB
       "Use a system-installed raylib instead of building it from source."
       OFF)

if(BUILD_FACTORY_USE_SYSTEM_RAYLIB)
  find_package(raylib ${RAYLIB_VERSION} QUIET)
endif()

include(FetchContent)

FetchContent_Declare(
  nlohmann_json
  DOWNLOAD_EXTRACT_TIMESTAMP OFF
  URL https://github.com/nlohmann/json/archive/refs/tags/${NLOHMANN_JSON_VERSION}.tar.gz
)

if(NOT raylib_FOUND)
  set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
  set(WITH_PIC ON CACHE BOOL "" FORCE)

  FetchContent_Declare(
    raylib
    DOWNLOAD_EXTRACT_TIMESTAMP OFF
    URL https://github.com/raysan5/raylib/archive/refs/tags/${RAYLIB_VERSION}.tar.gz
  )
  FetchContent_MakeAvailable(raylib)
endif()

FetchContent_MakeAvailable(nlohmann_json)

add_executable(game
  src/scripts/main/main.cpp
  src/scripts/renderer/renderer.cpp
  src/scripts/world/world.cpp
  src/scripts/player/player.cpp
)

target_compile_features(game PRIVATE cxx_std_17)

# ✅ FIX 1: Clean Up Include Management
# Instead of adding every single subfolder, point directly to the scripts root.
# This makes your source file headers simple and bulletproof!
target_include_directories(game PRIVATE src/scripts)

target_link_libraries(game PRIVATE raylib nlohmann_json::nlohmann_json)

# ✅ FIX 2: Automatic Asset Synchronization 
# Every time you compile, CMake will cleanly copy your source asset folder 
# directly into the build environment. This prevents texture & path loading crashes.
add_custom_command(TARGET game POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    "${CMAKE_CURRENT_SOURCE_DIR}/src/assets"
    "$<TARGET_FILE_DIR:game>/src/assets"
    COMMENT "Synchronizing game assets into target output folder..."
)

