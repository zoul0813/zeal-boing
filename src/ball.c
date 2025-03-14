#include <stdio.h>
#include <stdlib.h>
#include <zos_vfs.h>
#include <zos_sys.h>
#include <zos_video.h>
#include <zvb_gfx.h>
#include <zgdk.h>
#include <zgdk/tilemap/scroll.h>

#include "ball.h"
#include "assets.h"

#define EDGE_LEFT   (0)
#define EDGE_RIGHT  (SCREEN_WIDTH - (SPRITE_WIDTH * BALL_WIDTH))
#define EDGE_TOP    (0)
#define EDGE_BOTTOM (SCREEN_HEIGHT - (SPRITE_HEIGHT * BALL_HEIGHT))

#define VELOCITY_Y    4
#define VELOCITY_X    1

static gfx_context vctx;
static uint8_t frames = 0;
static int8_t direction_x = VELOCITY_X;
static uint16_t palette[PALETTE_SIZE];

static Vector2 ball = {
    .x = 16,
    .y = 0,
};


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
    if (direction_x > 0) {
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
    uint8_t y = 0;
    for (y = 0; y < BALL_HEIGHT; y++) {
        gfx_tilemap_load(&vctx, sphere_ztm[y], BALL_WIDTH, 1, WIDTH, HEIGHT + y);
    }
}


static void setup_grid(void)
{
    uint8_t y = 0;
    for(y = 0; y < HEIGHT; y++) {
        gfx_tilemap_load(&vctx, grid_ztm[y], WIDTH, 0, 0, y);
    }
}


static void bounce_ball(void)
{
    static int16_t acceleration = 0;

    ball.x += direction_x;
    ball.y += acceleration / 8;

    acceleration++;

    if(ball.y > EDGE_BOTTOM - 12) {
        acceleration = -acceleration + 2;
        /* Compensate the acceleration to make sure we don't hit the top */
        ball.y = EDGE_BOTTOM - 12;
    }

    if(ball.y < EDGE_TOP) {
        acceleration = -acceleration;
        ball.y = EDGE_TOP;
    }

    if (ball.x > EDGE_RIGHT) {
        direction_x = -VELOCITY_X;
        ball.x      = EDGE_RIGHT;
    }
    if (ball.x <= EDGE_LEFT) {
        direction_x = VELOCITY_X;
        ball.x      = EDGE_LEFT;
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

        for(uint8_t i = 0; i < abs(direction_x); i++)
            shift_palette();
        bounce_ball();

        gfx_wait_vblank(&vctx);
        /* Keep Y axis downward, so that Y + 1 is the line below Y + 0 */
        tilemap_scroll(1, SCREEN_WIDTH - ball.x, SCREEN_HEIGHT - ball.y);
        gfx_palette_load(&vctx, palette, sizeof(palette), PALETTE_INDEX);
        gfx_wait_end_vblank(&vctx);
    }
    deinit();
    return 0;
}
