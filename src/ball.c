#include <stdio.h>
#include <stdlib.h>
#include <zos_vfs.h>
#include <zos_sys.h>
#include <zos_video.h>
#include <zvb_gfx.h>
#include <zgdk.h>

#include "ball.h"
#include "assets.h"

#define BALL_HEIGHT 8
#define BALL_WIDTH  8

#define EDGE_LEFT   (0)
#define EDGE_RIGHT  (SCREEN_WIDTH - (SPRITE_WIDTH * BALL_WIDTH))
#define EDGE_TOP    (SCREEN_HEIGHT - (SPRITE_HEIGHT * BALL_HEIGHT))
#define EDGE_BOTTOM (0)

#define VELOCITY_Y    4
#define VELOCITY_X    1

static gfx_context vctx;
static uint8_t frames = 0;
static Point ball    = {
    .x = EDGE_RIGHT / 2,
    .y = EDGE_TOP - SPRITE_HEIGHT,
};
static Direction direction = {
    .x = VELOCITY_X,
    .y = VELOCITY_Y,
};
static uint16_t palette[PALETTE_SIZE];


static void deinit(void)
{
    tilemap_scroll(0, 0, 0);
    tilemap_scroll(1, 0, 0);

    // reset screen
    ioctl(DEV_STDOUT, CMD_RESET_SCREEN, NULL);
}


static void handle_error(zos_err_t err, const char* message, uint8_t fatal)
{
    if (err != ERR_SUCCESS) {
        if (fatal)
            deinit();
        printf("\nError[%d] (%02x) %s", err, err, message);
        if (fatal)
            exit(err);
    }
}


static void init(void)
{
    zos_err_t err;

    err = input_init(1);
    handle_error(err, "Failed to init input", 1);

    input_flush();

    gfx_enable_screen(0);

    err = gfx_initialize(ZVB_CTRL_VID_MODE_GFX_320_8BIT, &vctx);
    handle_error(err, "Failed to init graphics", 1);

    err = load_palette(&vctx);
    handle_error(err, "Failed to load palette", 1);

    err = load_sphere(&vctx);
    handle_error(err, "Failed to load sphere", 1);

    err = load_background(&vctx);
    handle_error(err, "Failed to load background", 0);

    gfx_enable_screen(1);
}


static void clear_layers(void)
{
    uint8_t y, x;
    uint8_t line[80];
    for (x = 0; x < 80; x++) {
        line[x] = 0;
    }
    for (y = 0; y < 40; y++) {
        gfx_tilemap_load(&vctx, line, 80, 0, 0, y);
        gfx_tilemap_load(&vctx, line, 80, 1, 0, y);
    }
}


static void setup_palette(void)
{
    uint8_t i;
    for (i = 0; i < PALETTE_SIZE / 2; i++) {
        palette[i] = WHITE;
    }
    for (; i < PALETTE_SIZE; i++) {
        palette[i] = RED;
    }

    gfx_palette_load(&vctx, palette, sizeof(palette), PALETTE_INDEX);
}


static void shift_palette (void)
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


static void setup_ball(void)
{
    static const uint8_t sphere_ztm[BALL_HEIGHT][BALL_WIDTH] = {
        { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07 },
        { 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f },
        { 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17 },
        { 0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f },
        { 0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27 },
        { 0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f },
        { 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37 },
        { 0x38, 0x39, 0x3a, 0x3b, 0x3c, 0x3d, 0x3e, 0x3f }
    };

    uint8_t y = 0;
    for (y = 0; y < BALL_HEIGHT; y++) {
        gfx_tilemap_load(&vctx, sphere_ztm[y], BALL_WIDTH, 1, WIDTH - BALL_WIDTH, HEIGHT - BALL_HEIGHT + y);
    }
}


static void setup_grid(void)
{
    static uint8_t grid_ztm[] = {
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02,
        0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x00, 0x00,
        0x00, 0x01, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02,
        0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x00, 0x00, 0x00, 0x01, 0x02, 0x02,
        0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02,
        0x02, 0x02, 0x00, 0x00, 0x00, 0x01, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02,
        0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x00, 0x00,
        0x00, 0x01, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02,
        0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x00, 0x00, 0x00, 0x01, 0x02, 0x02,
        0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02,
        0x02, 0x02, 0x00, 0x00, 0x00, 0x01, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02,
        0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x00, 0x00,
        0x00, 0x01, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02,
        0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x00, 0x00, 0x00, 0x01, 0x02, 0x02,
        0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02,
        0x02, 0x02, 0x00, 0x00, 0x00, 0x01, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02,
        0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x00, 0x00,
        0x00, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d,
        0x0e, 0x0f, 0x10, 0x11, 0x12, 0x13, 0x14, 0x00, 0x15, 0x16, 0x17, 0x18,
        0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f, 0x20, 0x21, 0x22, 0x23, 0x24,
        0x25, 0x26, 0x27, 0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f, 0x30,
        0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3a, 0x3b, 0x3c,
    };
    uint8_t* ptr = grid_ztm;

    for (uint16_t i = 0; i < sizeof(grid_ztm); i++) {
        *ptr = *ptr + GRID_TILE_FROM;
        ptr++;
    }

    ptr = grid_ztm;
    for (uint8_t i = 0; i < HEIGHT; i++) {
        gfx_tilemap_load(&vctx, ptr, 20, 0, 0, i);
        ptr += WIDTH;
    }
}


static void bounce_ball(void)
{
    static int16_t acceleration = 0;

    ball.x += direction.x;
    ball.y -= acceleration / 8;

    acceleration++;

    if(ball.y < VELOCITY_Y) {
        acceleration = -acceleration;
        /* Compensate the acceleration to make sure we don't hit the top */
        acceleration += 1;
        ball.y = 1;
    }

    if(ball.y > EDGE_TOP) {
        acceleration = -acceleration;
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
    setup_grid();
    uint16_t input = 0;
    while (1) {
        input = input_get();
        /* On the native emulator, using START returns directly (need to flush the input?) */
        if (BUTTON1_B)
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
