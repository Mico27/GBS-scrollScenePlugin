#ifndef SCENE_TRANSITION_H
#define SCENE_TRANSITION_H

#include <gbdk/platform.h>

#include "gbs_types.h"

#define DIRECTION_UP 1
#define DIRECTION_RIGHT 2
#define DIRECTION_DOWN 4
#define DIRECTION_LEFT 8
#define SCROLL_CAM_X 1280
#define SCROLL_CAM_Y 1152
#define SCROLL_CAM_SPEED 64
#define SCROLL_PLAYER_SPEED 8

extern UBYTE scene_transition_enabled;
extern UBYTE is_transitioning_scene;
extern far_ptr_t up_scene;
extern far_ptr_t right_scene;
extern far_ptr_t down_scene;
extern far_ptr_t left_scene;
extern UBYTE round_position_flags;

void enable_transition_to_scene(void) BANKED;
void scene_transition_reset(void) BANKED;
void check_transition_to_scene_collision(void) BANKED;
void transition_to_scene_modal(UBYTE direction) BANKED;
void transition_load_scene(UBYTE scene_bank, const scene_t * scene, BYTE t_scroll_x, BYTE t_scroll_y) BANKED;
uint8_t transition_camera_to(void) BANKED;
uint8_t transition_player_to(void) BANKED;

#endif
