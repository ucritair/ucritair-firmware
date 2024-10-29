
#define LCD_IMAGE_H 320
#define LCD_IMAGE_W 240
#define LCD_IMAGE_PIXELS (LCD_IMAGE_W * LCD_IMAGE_H)

extern uint16_t lcd_framebuffer[LCD_IMAGE_PIXELS];

void lcd_flip();
void lcd_init();
