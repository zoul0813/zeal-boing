#include <stdio.h>
#include <stdlib.h>
#include <zos_vfs.h>
#include <zos_sys.h>
#include <zos_video.h>
#include <zvb_gfx.h>
#include <zgdk.h>

#include "ball.h"
#include "assets.h"

#define BALL_HEIGHT 6
#define BALL_WIDTH  7

#define EDGE_LEFT   (0)
#define EDGE_RIGHT  (SCREEN_WIDTH - (SPRITE_WIDTH * 6))
#define EDGE_TOP    (SCREEN_HEIGHT - (SPRITE_HEIGHT * 6))
#define EDGE_BOTTOM (0)

#define VELOCITY_Y    4
#define VELOCITY_X    2

gfx_context vctx;
uint8_t frames = 0;

Point ball    = {
    .x = EDGE_RIGHT / 2,
    .y = EDGE_TOP - SPRITE_HEIGHT,
};
Direction direction = {
    .x = VELOCITY_X,
    .y = VELOCITY_Y,
};

static uint16_t palette[PALETTE_SIZE];

void handle_error(zos_err_t err, const char* message, uint8_t fatal)
{
    if (err != ERR_SUCCESS) {
        if (fatal)
            deinit();
        printf("\nError[%d] (%02x) %s", err, err, message);
        if (fatal)
            exit(err);
    }
}

void init(void)
{
    zos_err_t err;

    err = input_init(1);
    handle_error(err, "failed to init input", 1);

    gfx_enable_screen(0);

    err = gfx_initialize(ZVB_CTRL_VID_MODE_GFX_320_8BIT, &vctx);
    handle_error(err, "failed to init graphics", 1);

    err = load_palette(&vctx);
    handle_error(err, "Failed to load palette", 1);

    gfx_tileset_options options = {
        .compression = TILESET_COMP_RLE,
    };
    err = load_tileset(&vctx, &options);
    handle_error(err, "Failed to load tileset", 1);

    gfx_enable_screen(1);
}

void deinit(void)
{
    tilemap_scroll(0, 0, 0);
    tilemap_scroll(1, 0, 0);

    // reset screen
    ioctl(DEV_STDOUT, CMD_RESET_SCREEN, NULL);
}

void clear_layers(void)
{
    uint8_t y, x;
    uint8_t line0[80];
    uint8_t line1[80];
    for (y = 0; y < 40; y++) {
        for (x = 0; x < 80; x++) {
            line0[x] = 0;
            line1[x] = 0;
        }
        gfx_tilemap_load(&vctx, line0, 80, 0, 0, y);
        gfx_tilemap_load(&vctx, line1, 80, 1, 0, y);
    }
}

void setup_palette(void)
{
    uint8_t i;
    for (i = 0; i < PALETTE_SIZE / 2; i++) {
        palette[i] = WHITE;
    }
    for (; i < PALETTE_SIZE; i++) {
        palette[i] = RED;
    }

    gfx_palette_load(&vctx, palette, sizeof(uint8_t) * sizeof(palette), PALETTE_INDEX);
}

void shift_palette(void)
{
    uint8_t i;
    if (direction.x > 0) {
        uint16_t* p1        = &palette[PALETTE_SIZE - 1]; // last
        uint16_t* p2        = &palette[PALETTE_SIZE - 2]; // 2nd to last
        uint16_t last_color = *p1;
        for (i = 0; i < PALETTE_SIZE - 1; i++) {
            *(p1--) = *(p2--);
        }
        *p1 = last_color;
    } else {
        uint16_t* p1         = &palette[0];
        uint16_t* p2         = &palette[1];
        uint16_t first_color = *p1;
        for (i = 0; i < PALETTE_SIZE - 1; i++) {
            *(p1++) = *(p2++);
        }
        *p1 = first_color;
    }
}

void setup_ball(void)
{
    uint8_t x, y, i = 0;
    uint8_t line[BALL_WIDTH];
    for (y = 0; y < BALL_HEIGHT; y++) {
        for (x = 0; x < BALL_WIDTH; x++, i++) {
            line[x] = SPRITE_INDEX + i;
        }
        gfx_tilemap_load(&vctx, line, BALL_WIDTH, 1, WIDTH - BALL_HEIGHT, HEIGHT - BALL_HEIGHT + y);
    }
}

void bounce_ball(void)
{
    ball.x += direction.x;
    ball.y += direction.y;

    if(ball.y < VELOCITY_Y) {
        direction.y = VELOCITY_Y;
        ball.y = 1;
    }
    if(ball.y > EDGE_TOP) {
        direction.y = -VELOCITY_Y;
        ball.y = EDGE_TOP - VELOCITY_Y;
    }

    if (ball.x > EDGE_RIGHT) {
        direction.x = -VELOCITY_X;
        ball.x      = EDGE_RIGHT - VELOCITY_X;
    }
    if (ball.x < VELOCITY_X) {
        direction.x = VELOCITY_X;
        ball.x      = VELOCITY_X;
    }
}

int main(void)
{
    init();
    setup_palette();
    clear_layers();
    setup_ball();
    uint16_t input = 0;
    while (1) {
        input = input_get();
        if (START1)
            break;

        frames++;

        for(uint8_t i = 0; i < abs(direction.x); i++)
            shift_palette();
        bounce_ball();

        gfx_wait_vblank(&vctx);
        tilemap_scroll(1, ball.x, ball.y);
        gfx_palette_load(&vctx, palette, sizeof(palette), PALETTE_INDEX);
        gfx_wait_end_vblank(&vctx);
    }
    deinit();
    return 0;
}
