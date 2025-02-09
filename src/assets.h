#include <stdint.h>
#include <zos_errors.h>
#include <zvb_gfx.h>
// #include <sound/tracker.h>

/* Load the grid tiles from tile 128 */
#define GRID_TILE_FROM  128U

gfx_error load_palette(gfx_context* ctx);
gfx_error load_sphere(gfx_context* ctx);
gfx_error load_background(gfx_context* ctx);
// zos_err_t load_zmt(track_t* track, uint8_t index);
