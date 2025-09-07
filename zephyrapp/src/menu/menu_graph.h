#include "cat_machine.h"
#include "lcd_driver.h"

void CAT_MS_graph(CAT_FSM_signal signal);
void CAT_render_graph();

#define GRAPH_PAD 6
#define GRAPH_MARGIN 2
#define GRAPH_W (LCD_IMAGE_W-(GRAPH_PAD*2)-(GRAPH_MARGIN*2))
#define GRAPH_H 120
