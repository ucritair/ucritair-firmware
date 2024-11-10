#include <stdint.h>

#define CAT_BTN_MASK_SELECT 0x01
#define CAT_BTN_MASK_START 0x02
#define CAT_BTN_MASK_A 0x04
#define CAT_BTN_MASK_B 0x08
#define CAT_BTN_MASK_DOWN 0x10
#define CAT_BTN_MASK_RIGHT 0x20
#define CAT_BTN_MASK_LEFT 0x40
#define CAT_BTN_MASK_UP 0x80

void init_buttons();
void deinit_buttons();

extern uint8_t current_buttons;
void update_buttons();
