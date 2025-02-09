#include <stdio.h>
#include <zvb_gfx.h>
#include "assets.h"


extern uint8_t _palette_start;
extern uint8_t _palette_end;
extern uint8_t _ball_tileset_start;
extern uint8_t _ball_tileset_end;
extern uint8_t _grid_tileset_start;
extern uint8_t _grid_tileset_end;
// extern uint8_t _zmt_track1_start;
// extern uint8_t _zmt_track1_end;


gfx_error load_palette(gfx_context* ctx)
{
    /* load both the ball and the background palettes */
    const size_t palette_size = &_palette_end - &_palette_start;
    return gfx_palette_load(ctx, &_palette_start, palette_size, 0);
}


gfx_error load_sphere(gfx_context* ctx)
{
    static gfx_tileset_options options = {
        .compression = TILESET_COMP_RLE,
    };
    gfx_error err = gfx_tileset_load(ctx, &_ball_tileset_start, &_ball_tileset_end - &_ball_tileset_start, &options);

    return err;
}

gfx_error load_background(gfx_context* ctx) {
    const uint8_t palette_count = (&_palette_end - &_palette_start) / 2;
    static gfx_tileset_options options = {
        .compression = TILESET_COMP_1BIT,
        /* Load the grid from tile GRID_TILE_FROM */
        .from_byte = GRID_TILE_FROM << 8u,
        /* the last two colors in the palette are for the grid */
    };
    gfx_error err = gfx_tileset_load(ctx, &_grid_tileset_start, &_grid_tileset_end - &_grid_tileset_start, &options);
    return err;
}


static void __assets__(void) __naked
{
    __asm__(
        "__palette_start:\n"
        "    .incbin \"assets/sphere.ztp\"\n"
        "__palette_end:\n"

        "__ball_tileset_start:\n"
        "    .incbin \"assets/sphere.zts\"\n"
        "__ball_tileset_end:\n"

        "__grid_tileset_start:\n"
        "    .incbin \"assets/grid.zts\"\n"
        "__grid_tileset_end:\n"

        // background music
        // Track 1 - Space Battle
        // "__zmt_track1_start:\n"
        // "    .incbin \"assets/shooter.zmt\"\n"
        // "__zmt_track1_end:\n"
    );
}
