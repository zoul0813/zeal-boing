cmake_minimum_required(VERSION 3.16)

if(NOT EXISTS "$ENV{ZOS_PATH}/cmake/zos_init.cmake")
    message(FATAL_ERROR "Set ZOS_PATH to a Zeal OS checkout with the CMake toolchain, or build with zde cmake.")
endif()

set(ZOS_TOOLCHAIN sdcc)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/bin")
include("$ENV{ZOS_PATH}/cmake/zos_init.cmake")

project(zeal_boing LANGUAGES C ASM)

zos_use(zvb)
find_package(ZVB_SDK REQUIRED)
zos_use(zgdk)
find_package(ZGDK REQUIRED)
find_package(Python3 REQUIRED COMPONENTS Interpreter)

add_executable(boing src/ball.c src/assets.c)
target_link_libraries(boing PRIVATE zgdk zvb_gfx)

# Keep the existing filename-encoded bit depth, palette, strip, and RLE flags.
set(converter "$ENV{ZVB_SDK_PATH}/tools/zeal2gif/gif2zeal.py")
set(asset_dir "${CMAKE_CURRENT_BINARY_DIR}/assets")
set(asset_outputs)
foreach(asset grid__B1S03 sphere__B8C12S00Z)
    string(REGEX REPLACE "__.*$" "" name "${asset}")
    set(tiles "${asset_dir}/${name}.zts")
    set(palette "${asset_dir}/${name}.ztp")
    add_custom_command(
        OUTPUT "${tiles}" "${palette}"
        COMMAND "${CMAKE_COMMAND}" -E make_directory "${asset_dir}"
        COMMAND "${Python3_EXECUTABLE}" "${converter}"
            -i "${CMAKE_CURRENT_SOURCE_DIR}/assets/${asset}.gif" -o "${asset_dir}"
        DEPENDS "assets/${asset}.gif" "${converter}"
        VERBATIM
    )
    list(APPEND asset_outputs "${tiles}" "${palette}")
endforeach()
add_custom_target(boing_assets DEPENDS ${asset_outputs})
add_dependencies(boing boing_assets)
set_property(SOURCE src/assets.c APPEND PROPERTY OBJECT_DEPENDS "${asset_outputs}")
zos_add_outputs(boing)
