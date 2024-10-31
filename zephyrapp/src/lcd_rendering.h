#include <stdint.h>

void lcd_render_diag();

void lcd_write_char(uint16_t color, int x, int y, char c);
void lcd_write_str(uint16_t color, int x, int y, char* str);
