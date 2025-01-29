#include <stdio.h>
#include <zos_vfs.h>
#include <zos_sys.h>
#include <zos_video.h>
#include <zvb_gfx.h>
// #include <zgdk.h>
#include "input.h"
#include "misc.h"

#include "ball.h"
#include "assets.h"

gfx_context vctx;
uint8_t frames = 0;
Direction direction = { .x = DIRECTION_LEFT, .y = DIRECTION_DOWN };
gfx_sprite sprites[SPRITE_COUNT];
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
    // reset screen
    ioctl(DEV_STDOUT, CMD_RESET_SCREEN, NULL);
}

void load_tilemap(void)
{
    uint8_t y, x;
    uint8_t line0[WIDTH];
    uint8_t line1[WIDTH];
    for (y = 0; y < HEIGHT; y++) {
        // uint8_t ball1 = (y & 0x1) == 0 ? BALL1 : BALL2;
        // uint8_t ball2 = (y & 0x1) == 0 ? BALL2 : BALL1;
        for (x = 0; x < WIDTH; x++) {
            // line0[x] = (x & 0x01) == 0 ? ball1 : ball2;
            // line0[x] = BALL1;
            line0[x] = 0;
            line1[x] = 0;
        }
        gfx_tilemap_load(&vctx, line0, WIDTH, 0, 0, y);
        gfx_tilemap_load(&vctx, line1, WIDTH, 1, 0, y);
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
    switch(direction.x) {
        case DIRECTION_LEFT: {
            uint16_t* p1 = &palette[0];
            uint16_t* p2 = &palette[1];
            uint16_t first_color = *p1;
            for (i = 0; i < PALETTE_SIZE - 1; i++) {
                *(p1++) = *(p2++);
            }
            *p1 = first_color;
        } break;
        case DIRECTION_RIGHT: {
            uint16_t* p1 = &palette[PALETTE_SIZE - 1]; // last
            uint16_t* p2 = &palette[PALETTE_SIZE - 2]; // 2nd to last
            uint16_t last_color = *p1;
            for (i = PALETTE_SIZE; i > 0; i--) {
                *(p1--) = *(p2--);
            }
            *p1 = last_color;
        } break;
    }
}

void setup_ball(void) {
    uint8_t x = 0, x_offset = (SCREEN_WIDTH / 4) + (SPRITE_WIDTH);
    uint8_t y = 0, y_offset = (SCREEN_HEIGHT / 4);

    gfx_sprite* sprite = &sprites[0];
    for (uint8_t i = 0; i < SPRITE_COUNT; i++) {
        if((i % 7) == 0) {
            x = 0;
            y += SPRITE_HEIGHT;
        }

        sprite->flags      = SPRITE_NONE;
        sprite->tile       = SPRITE_INDEX + i;
        sprite->x          = x_offset + x;
        sprite->y          = y_offset + y;

        x += SPRITE_WIDTH;

        sprite++;
    }
    gfx_sprite_render_array(&vctx, 0, sprites, SPRITE_COUNT);
}

void bounce_ball(void) {
    gfx_sprite* sprite = &sprites[0];
    uint16_t x = sprite->x;
    uint16_t y = sprite->y;

    if(x < 17) direction.x = DIRECTION_RIGHT;
    if(x > SCREEN_WIDTH - (SPRITE_WIDTH * 5)) direction.x = DIRECTION_LEFT;

    if(y < 17) direction.y = DIRECTION_DOWN;
    if(y > SCREEN_HEIGHT - (SPRITE_HEIGHT * 5)) direction.y = DIRECTION_UP;

    for (uint8_t i = 0; i < SPRITE_COUNT; i++) {
        sprite->x += direction.x;
        sprite->y += direction.y;
        sprite++;
    }
}

int main(void)
{
    init();
    setup_palette();
    setup_ball();
    load_tilemap();
    uint16_t input = 0;
    while (1) {
        input = input_get();
        if (START1)
            break;

        frames++;
        // TSTATE_LOG(2);
        shift_palette();
        bounce_ball();
        // TSTATE_LOG(2);
        gfx_wait_vblank(&vctx);
        // TSTATE_LOG(1);
        gfx_palette_load(&vctx, palette, sizeof(palette), PALETTE_INDEX);
        // sprites_render(&vctx);
        gfx_sprite_render_array(&vctx, 0, sprites, SPRITE_COUNT);
        // TSTATE_LOG(1);
        gfx_wait_end_vblank(&vctx);
    }
    deinit();
    return 0;
}
