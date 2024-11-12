#include <stdint.h>

#ifdef MINIMIZE_GAME_FOOTPRINT
#define LCD_IMAGE_H 128
#define LCD_IMAGE_W 128
#else
#define LCD_IMAGE_H 320
#define LCD_IMAGE_W 240
#endif

#define LCD_FRAMEBUFFER_SEGMENTS 10
#define LCD_FRAMEBUFFER_W LCD_IMAGE_W
#define LCD_FRAMEBUFFER_H (LCD_IMAGE_H/LCD_FRAMEBUFFER_SEGMENTS)

#define LCD_IMAGE_PIXELS (LCD_IMAGE_W * LCD_IMAGE_H)
#define LCD_FRAMEBUFFER_PIXELS (LCD_FRAMEBUFFER_W * LCD_FRAMEBUFFER_H)

extern uint16_t* lcd_framebuffer;

#define LCD_FRAMEBUFFER_A_B 1
#ifdef LCD_FRAMEBUFFER_A_B
extern uint16_t lcd_framebuffer_pair[LCD_FRAMEBUFFER_PIXELS][2];
#else
extern uint16_t lcd_framebuffer_backing[LCD_FRAMEBUFFER_PIXELS];
#endif

extern int framebuffer_offset_h;

void lcd_flip(uint16_t* buffer, int offset);
void lcd_init();
void set_backlight(int pct);

#define BACKLIGHT_FULL 60
#define BACKLIGHT_DIM 5