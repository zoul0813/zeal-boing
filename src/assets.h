#include <stdint.h>
#include <zos_errors.h>
#include <zvb_gfx.h>
// #include <sound/tracker.h>

void __assets__(void);

extern uint8_t _palette_end;
extern uint8_t _palette_start;
extern uint8_t _tileset_end;
extern uint8_t _tileset_start;
// extern uint8_t _zmt_track1_start;
// extern uint8_t _zmt_track1_end;

gfx_error load_palette(gfx_context* ctx);
gfx_error load_tileset(gfx_context* ctx, gfx_tileset_options* options);
// zos_err_t load_zmt(track_t* track, uint8_t index);
