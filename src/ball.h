
#define SCREEN_MODE   ZVB_CTRL_VID_MODE_GFX_320_8BIT
#define SCREEN_WIDTH  320
#define SCREEN_HEIGHT 240
#define WIDTH         20
#define HEIGHT        15
#define SPRITE_HEIGHT       16
#define SPRITE_WIDTH        16

// #define LEFT1     (input1 & BUTTON_LEFT)
// #define RIGHT1    (input1 & BUTTON_RIGHT)
// #define UP1       (input1 & BUTTON_UP)
// #define DOWN1     (input1 & BUTTON_DOWN)
// #define BUTTON1_A (input1 & BUTTON_A)
#define BUTTON1_B (input & BUTTON_B)
#define START1    (input & BUTTON_START)
// #define SELECT1   (input1 & BUTTON_SELECT)

#define BALL1 1
#define BALL2 2

#define PALETTE_INDEX 4
#define PALETTE_SIZE  32
// #define PALETTE_SIZE  16
#define WHITE 0xFFFF
#define RED   0xF800

#define SPRITE_INDEX    7
#define SPRITE_COUNT    42


void init(void);
void deinit(void);
