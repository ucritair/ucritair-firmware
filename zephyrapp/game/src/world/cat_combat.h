#pragma once

void CAT_spawn_enemy(int x, int y);

void CAT_tick_enemies();
void CAT_render_enemies();

void CAT_attack_bullet(int x, int y, int tx, int ty, int speed);
void CAT_attack_swipe(int x, int y, int tx, int ty);

void CAT_tick_attacks();
void CAT_render_attacks();