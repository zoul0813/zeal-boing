#include <stdint.h>
#include <zos_errors.h>
#include <zvb_gfx.h>
// #include <sound/tracker.h>

/* Load the grid tiles from tile 128 */
#define GRID_TILE_FROM  128U
#define BALL_HEIGHT     8
#define BALL_WIDTH      8

extern const uint8_t sphere_ztm[BALL_HEIGHT][BALL_WIDTH];
extern const uint8_t grid_ztm[15][20];

gfx_error load_palette(gfx_context* ctx);
gfx_error load_sphere(gfx_context* ctx);
gfx_error load_background(gfx_context* ctx);
// zos_err_t load_zmt(track_t* track, uint8_t index);

