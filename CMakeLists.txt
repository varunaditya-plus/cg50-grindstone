# Configure with [fxsdk build-fx] or [fxsdk build-cg], which provide the
# toolchain file and module path of the fxSDK

cmake_minimum_required(VERSION 3.15)
project(Grindstone)

include(GenerateG1A)
include(GenerateG3A)
include(GenerateHH2Bin)
include(Fxconv)
find_package(Gint 2.9 REQUIRED)

set(SOURCES
  src/main.c
  src/screens.c
  src/game.h
  src/grid.c
  src/grid.h
  src/monsters.c
  src/monsters.h
  src/player.c
  src/player.h
  src/chain.c
  src/chain.h
  src/grindstone.c
  src/grindstone.h
  src/font.c
  src/font.h
  src/powerups.c
  src/powerups.h
  src/screens.h
  src/bosses.c
  src/bosses.h
  src/levels.c
  src/levels.h
  src/objects.c
  src/objects.h
)
# Shared assets, fx-9860G-only assets and fx-CG-50-only assets
set(ASSETS
  # ...
)
set(ASSETS_fx
  assets-fx/example.png
  # ...
)
set(ASSETS_cg
  assets-cg/example.png
  # ...
)

fxconv_declare_assets(${ASSETS} ${ASSETS_fx} ${ASSETS_cg} WITH_METADATA)

add_executable(myaddin ${SOURCES} ${ASSETS} ${ASSETS_${FXSDK_PLATFORM}})
target_compile_options(myaddin PRIVATE -Wall -Wextra -Os -g -flto)
target_link_libraries(myaddin Gint::Gint)

if("${FXSDK_PLATFORM_LONG}" STREQUAL fx9860G)
  generate_g1a(TARGET myaddin OUTPUT "Grindstone.g1a"
    NAME "Grindstone" ICON assets-fx/icon.png)
elseif("${FXSDK_PLATFORM_LONG}" STREQUAL fxCG50)
  generate_g3a(TARGET myaddin OUTPUT "Grindstone.g3a"
    NAME "Grindstone" ICONS assets-cg/icon-uns.png assets-cg/icon-sel.png)
elseif("${FXSDK_PLATFORM_LONG}" STREQUAL fx9860G_G3A)
  generate_g3a(TARGET myaddin OUTPUT "Grindstone-fx.g3a"
    NAME "Grindstone-fx" ICONS assets-cg/icon-uns.png assets-cg/icon-sel.png)
elseif("${FXSDK_PLATFORM_LONG}" STREQUAL fxCP)
  generate_hh2_bin(TARGET myaddin OUTPUT "Grindstone-hh2.bin")
endif()
