#pragma once

typedef struct {
    double pos_x, pos_y;
    double dir_x, dir_y;
    double plane_x, plane_y;
} Player;

void player_update(Player* p, double dt);
void player_rotate(Player* p, double angle);