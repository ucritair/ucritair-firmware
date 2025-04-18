 #include <stdlib.h>                 /* rand() */
 #include "cat_arcade.h"
 #include "cat_machine.h"
 #include "cat_input.h"
 #include "cat_render.h"
 #include "cat_gui.h"
 #include "sprite_assets.h"
 
 /* forward decls so the linker can see the symbols early */
 void CAT_MS_brick(CAT_machine_signal signal);
 void CAT_render_brick(void);
 void CAT_MS_room(CAT_machine_signal signal);
 
 /* -------------------------------------------------- sprites */
 static const CAT_sprite *brick_sprites[5] = {
     &ash_floor_sprite,
     &grass_floor_sprite,
     &base_floor_sprite,
     &base_wall_sprite,
     &sky_wall_sprite
 };
 #define PADDLE_SPRITE  snake_body_sprite
 #define PADDLE_FRAME   0
 #define BALL_SPRITE    mines_sprite
 #define BALL_FRAME     10
 
 /* -------------------------------------------------- tunables */
 #define GRID_W         15
 #define GRID_H         20
 #define TILE           CAT_TILE_SIZE   /* 16 px */
 
 #define PADDLE_Y       (GRID_H - 2)
 #define PADDLE_W       4      /* 4 tiles ⇒ 64 px */
 #define PADDLE_STEP    1      /* tiles per tick */
 
 #define BALL_TICK      1     /* make this 2 to make the game easier (ball slower) */
 #define BRICK_ROWS     5
 #define BRICK_COLS     GRID_W
 
 #define RNG()          (rand())
 
 /* -------------------------------------------------- game state */
 struct ball_t {
     int x, y;     /* tile */
     int fx, fy;   /* 0‑15 sub‑tile fraction */
     int dx, dy;   /* direction −1 / +1 */
 } ball;
 
 static int  paddle_x;                       /* leftmost tile */
 static bool bricks[BRICK_ROWS][BRICK_COLS];
 static int  bricks_left;
 static int  score, hi_score;
 static int  tick_ctr;
 static bool game_over, win;
 
 /* -------------------------------------------------- helpers */
 static void reset_ball(void)
 {
     ball.x = GRID_W / 2;
     ball.y = PADDLE_Y - 1;
     ball.fx = ball.fy = 0;
     ball.dx = 1;
     ball.dy = -1;
 }
 
 static void level_init(void)
 {
     for (int r = 0; r < BRICK_ROWS; ++r)
         for (int c = 0; c < BRICK_COLS; ++c)
             bricks[r][c] = true;
 
     bricks_left = BRICK_ROWS * BRICK_COLS;
     score       = 0;
     paddle_x    = (GRID_W - PADDLE_W) / 2;
     game_over   = false;
     win         = false;
     tick_ctr    = 0;
     reset_ball();
 }
 
 /* -------------------------------------------------- state machine */
 void CAT_MS_brick(CAT_machine_signal sig)
 {
     switch (sig) {
     case CAT_MACHINE_SIGNAL_ENTER:
         CAT_set_render_callback(CAT_render_brick);
         level_init();
         break;
 
     case CAT_MACHINE_SIGNAL_TICK:
         /* ---------------- global hotkeys */
         if (CAT_input_pressed(CAT_BUTTON_B))        CAT_machine_back();
         if (CAT_input_pressed(CAT_BUTTON_START))    CAT_machine_transition(CAT_MS_room);
 
         if (game_over) {
             if (CAT_input_pressed(CAT_BUTTON_A))    CAT_machine_back();
             break; /* freeze until exit */
         }
 
         /* ---------------- paddle input */
         if (CAT_input_held(CAT_BUTTON_LEFT, 0.0f) && paddle_x > 0)
             paddle_x -= PADDLE_STEP;
         if (CAT_input_held(CAT_BUTTON_RIGHT, 0.0f) && paddle_x < GRID_W - PADDLE_W)
             paddle_x += PADDLE_STEP;
 
         /* clamp */
         if (paddle_x < 0)                 paddle_x = 0;
         if (paddle_x > GRID_W - PADDLE_W) paddle_x = GRID_W - PADDLE_W;
 
         /* ---------------- ball physics */
         if (++tick_ctr >= BALL_TICK) {
             tick_ctr = 0;
             ball.fx += ball.dx * 16;
             ball.fy += ball.dy * 16;
 
             while (ball.fx < 0)   { ball.fx += 16; --ball.x; }
             while (ball.fx >= 16) { ball.fx -= 16; ++ball.x; }
             while (ball.fy < 0)   { ball.fy += 16; --ball.y; }
             while (ball.fy >= 16) { ball.fy -= 16; ++ball.y; }
 
             /* walls */
             if (ball.x < 0)           { ball.x = 0;        ball.dx = -ball.dx; }
             if (ball.x >= GRID_W)     { ball.x = GRID_W-1; ball.dx = -ball.dx; }
             if (ball.y < 0)           { ball.y = 0;        ball.dy = -ball.dy; }
 
             /* paddle – sub‑tile collision so edges never clip */
             if (ball.dy > 0 && ball.y >= PADDLE_Y - 1) {
                 int bx_px  = ball.x * 16 + ball.fx;                 /* ball X in pixels */
                 int by_px  = ball.y * 16 + ball.fy;                 /* ball Y in pixels */
                 int pad0px = paddle_x * 16;                         /* paddle span */
                 int pad1px = (paddle_x + PADDLE_W) * 16;
 
                 /* check overlap with the paddle surface */
                 if (by_px >= (PADDLE_Y * 16) - 1 && bx_px >= pad0px && bx_px < pad1px) {
                     /* bounce */
                     ball.y  = PADDLE_Y - 1;
                     ball.fy = 0;
                     ball.dy = -1;
 
                     int rel_px = bx_px - pad0px;                    /* 0..(PADDLE_W*16-1) */
                     int quarter = rel_px / (16 * (PADDLE_W / 4));   /* 0,1,2,3 */
                     if (quarter == 0)      ball.dx = -1;
                     else if (quarter == 3) ball.dx =  1;
                     else                   ball.dx = (RNG() & 1) ? -1 : 1;
                 }
             }
 
             /* bricks */
             if (ball.y < BRICK_ROWS) {
                 int r = ball.y, c = ball.x;
                 if (bricks[r][c]) {
                     bricks[r][c] = false;
                     --bricks_left;
                     score += 10;
 
                     int flip = RNG() & 3; /* 0..3 */
                     if (flip & 1) ball.dx = -ball.dx;
                     if (flip & 2) ball.dy = -ball.dy;
                     if (!flip)    ball.dy = -ball.dy; /* never zero‑flip */
 
                     if (bricks_left == 0) { win = true; game_over = true; }
                 }
             }
 
             /* bottom‑out */
             if (ball.y >= GRID_H) { game_over = true; win = false; }
         }
         break;
 
     case CAT_MACHINE_SIGNAL_EXIT:
         if (score > hi_score) hi_score = score;
         break;
     }
 }
 
 /* -------------------------------------------------- rendering */
 void CAT_render_brick(void)
 {
     draw_mode = CAT_DRAW_MODE_DEFAULT;
     CAT_frameberry(RGB8882565(0,0,32));
 
     /* bricks */
     for (int r = 0; r < BRICK_ROWS; ++r)
         for (int c = 0; c < BRICK_COLS; ++c)
             if (bricks[r][c])
                 CAT_draw_sprite(brick_sprites[r], 0, c * TILE, r * TILE);
 
     /* paddle */
     for (int i = 0; i < PADDLE_W; ++i)
         CAT_draw_sprite(&PADDLE_SPRITE, PADDLE_FRAME,
                         (paddle_x + i) * TILE, PADDLE_Y * TILE);
 
     /* ball */
     CAT_draw_sprite(&BALL_SPRITE, BALL_FRAME, ball.x * TILE, ball.y * TILE);
 
     /* HUD */
     CAT_gui_textf("Score:%d  Hi:%d\n", score, hi_score);
 
     if (game_over) {
         CAT_gui_panel((CAT_ivec2){1, 7}, (CAT_ivec2){13, 11});
         CAT_gui_text(win ? "  STAGE CLEARED!\n\n" : "  GAME OVER\n\n");
         CAT_gui_text("Press A/B\n");
     }
 }
 