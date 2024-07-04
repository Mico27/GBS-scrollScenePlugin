#pragma bank 255

#include "scene_transition.h"

#include <string.h>

#include "system.h"
#include "actor.h"
#include "camera.h"
#include "data_manager.h"
#include "game_time.h"
#include "math.h"
#include "fade_manager.h"
#include "parallax.h"
#include "scroll.h"
#include "ui.h"
#include "bankdata.h"
#include "input.h"
#include "projectiles.h"
#include "shadow.h"
#include "music_manager.h"

#define TILE_FRACTION_MASK         0b1111111
#define ONE_TILE_DISTANCE          128

UBYTE scene_transition_enabled;
UBYTE is_transitioning_scene;
far_ptr_t up_scene;
far_ptr_t right_scene;
far_ptr_t down_scene;
far_ptr_t left_scene;
UBYTE round_position_flags;
UINT16 transitioning_player_pos_x;
UINT16 transitioning_player_pos_y;

void enable_transition_to_scene(void) BANKED {
	camera_settings &= ~(CAMERA_LOCK_FLAG);
	camera_x = SCROLL_CAM_X;
	camera_y = SCROLL_CAM_Y;
	scene_transition_enabled = 1;
}

void scene_transition_reset(void) BANKED {
	up_scene.bank = 0;
	right_scene.bank = 0;
	down_scene.bank = 0;
	left_scene.bank = 0;
	scene_transition_enabled = 0;
	transitioning_player_pos_x = 0xFFF;
	transitioning_player_pos_y = 0xFFF;
}

void check_transition_to_scene_collision(void) BANKED {	
	if (scene_transition_enabled && !PLAYER.disabled) {
		// Check for scene scroll
		if (transitioning_player_pos_y != PLAYER.pos.y)
		{
			transitioning_player_pos_y = 0xFFF;
			if ((PLAYER.pos.y) > 0xFFF || (PLAYER.pos.y) < 128){
				transition_to_scene_modal(DIRECTION_UP);				
			} else if (PLAYER.pos.y >= ((image_tile_height * 128) - 128)){
				transition_to_scene_modal(DIRECTION_DOWN);		
			}
		}
		if (transitioning_player_pos_x != PLAYER.pos.x)
		{
			transitioning_player_pos_x = 0xFFF;
			if ((PLAYER.pos.x) > 0xFFF){
				transition_to_scene_modal(DIRECTION_LEFT);
			} else if (PLAYER.pos.x >= ((image_tile_width * 128) - 256)){
				transition_to_scene_modal(DIRECTION_RIGHT);
			}
		}
	}
}

void transition_to_scene_modal(UBYTE direction) BANKED {
	UBYTE scene_bank;
	const scene_t * scene;
	if (direction == DIRECTION_UP){
		scene_bank = up_scene.bank;
		scene = up_scene.ptr;
	} else if (direction == DIRECTION_RIGHT){
		scene_bank = right_scene.bank;
		scene = right_scene.ptr;
	} else if (direction == DIRECTION_DOWN){
		scene_bank = down_scene.bank;
		scene = down_scene.ptr;
	} else {
		scene_bank = left_scene.bank;
		scene = left_scene.ptr;
	}
	if (scene_bank && scene){
		is_transitioning_scene = 1;
		transition_load_scene(scene_bank, scene, (direction == DIRECTION_RIGHT)? image_tile_width: (direction == DIRECTION_LEFT)? -image_tile_width: 0, (direction == DIRECTION_DOWN)? image_tile_height: (direction == DIRECTION_UP)? -image_tile_height: 0);
		transitioning_player_pos_x = PLAYER.pos.x + ((direction == DIRECTION_RIGHT)? 256: (direction == DIRECTION_LEFT)? -256: 0);
		transitioning_player_pos_y = PLAYER.pos.y + ((direction == DIRECTION_DOWN)? 256: (direction == DIRECTION_UP)? -256: 0);
		if (round_position_flags & direction){		
			transitioning_player_pos_x = (transitioning_player_pos_x  & ~TILE_FRACTION_MASK);
			transitioning_player_pos_y = (transitioning_player_pos_y  & ~TILE_FRACTION_MASK);
			if (direction == DIRECTION_UP){
				transitioning_player_pos_y += ONE_TILE_DISTANCE;
			} else if (direction == DIRECTION_LEFT){
				transitioning_player_pos_x += ONE_TILE_DISTANCE;
			}
		}
		if (direction == DIRECTION_DOWN){
			scroll_render_rows(draw_scroll_x, draw_scroll_y, 0, 2);
		} else if (direction == DIRECTION_LEFT){
			scroll_queue_col((scroll_x >> 3) - 1, 0);
		} else if (direction == DIRECTION_UP){
			scroll_queue_row(0, (scroll_y >> 3) - 1);
		}
		do {
			uint8_t camera_arrived = transition_camera_to();
			uint8_t player_arrived = transition_player_to();
			if (camera_arrived && player_arrived) {
				is_transitioning_scene = 0;
			}
			ui_update();
	
			toggle_shadow_OAM();
			camera_update();
			scroll_update();
			actors_update();
			projectiles_render();
			activate_shadow_OAM();
	
			game_time++;
			wait_vbl_done();
			input_update();
		} while (is_transitioning_scene);
	}
}

void transition_load_scene(UBYTE scene_bank, const scene_t * scene, BYTE t_scroll_x, BYTE t_scroll_y) BANKED {	
	initial_camera_x = camera_x = (-t_scroll_x * 128) + SCROLL_CAM_X;
	initial_camera_y = camera_y = (-t_scroll_y * 128) + SCROLL_CAM_Y;
	initial_player_x_pos = (PLAYER.pos.x -= (t_scroll_x * 128));
	initial_player_y_pos = (PLAYER.pos.y -= (t_scroll_y * 128));	
	bkg_offset_x = (bkg_offset_x + t_scroll_x) % 32;
	bkg_offset_y = (bkg_offset_y + t_scroll_y) % 32;	
    // kill all threads, but don't clear variables 
    script_runner_init(FALSE);
    // reset timers on scene change
    timers_init(FALSE);
    // reset input events on scene change
    events_init(FALSE);
    // reset music events
    music_init_events(FALSE);
	load_scene(scene, scene_bank, TRUE);	
}

uint8_t transition_camera_to(void) BANKED {
	// Move camera towards destination
	
	if ((camera_x == SCROLL_CAM_X) && (camera_y == SCROLL_CAM_Y)) {
        return TRUE;
    }
    if (camera_x > SCROLL_CAM_X) {
        // Move left
        camera_x -= SCROLL_CAM_SPEED;
        if (camera_x <= SCROLL_CAM_X) {
            camera_x = SCROLL_CAM_X;
        }
    } else if (camera_x < SCROLL_CAM_X) {
        // Move right
        camera_x += SCROLL_CAM_SPEED;
        if (camera_x >= SCROLL_CAM_X) {
            camera_x = SCROLL_CAM_X;
        }        
    }

    if (camera_y > SCROLL_CAM_Y) {
        // Move up
        camera_y -= SCROLL_CAM_SPEED;
        if (camera_y <= SCROLL_CAM_Y) {
            camera_y = SCROLL_CAM_Y;
        }        
    } else if (camera_y < SCROLL_CAM_Y) {
        // Move down
        camera_y += SCROLL_CAM_SPEED;
        if (camera_y >= SCROLL_CAM_Y) {
            camera_y = SCROLL_CAM_Y;
        }      
    }	
	if ((camera_x == SCROLL_CAM_X) && (camera_y == SCROLL_CAM_Y)) {
        return TRUE;
    }	
	return FALSE;
}


uint8_t transition_player_to(void) BANKED {
	
	if ((PLAYER.pos.x == transitioning_player_pos_x) && (PLAYER.pos.y == transitioning_player_pos_y)) {
        return TRUE;
    }
	
	UINT16 oldPlayerPosX = PLAYER.pos.x + SCROLL_CAM_X;
	UINT16 oldPlayerPosY = PLAYER.pos.y + SCROLL_CAM_Y;
	UINT16 newPlayerPosX = transitioning_player_pos_x + SCROLL_CAM_X;
	UINT16 newPlayerPosY = transitioning_player_pos_y + SCROLL_CAM_Y;
	
	if (oldPlayerPosX > newPlayerPosX) {
        // Move left
        oldPlayerPosX -= SCROLL_PLAYER_SPEED;
        if (oldPlayerPosX <= newPlayerPosX) {
            oldPlayerPosX = newPlayerPosX;
        }
    } else if (oldPlayerPosX < newPlayerPosX) {
        // Move right
        oldPlayerPosX += SCROLL_PLAYER_SPEED;
        if (oldPlayerPosX >= newPlayerPosX) {
            oldPlayerPosX = newPlayerPosX;
        }        
    }

    if (oldPlayerPosY > newPlayerPosY) {
        // Move up
        oldPlayerPosY -= SCROLL_PLAYER_SPEED;
        if (oldPlayerPosY <= newPlayerPosY) {
            oldPlayerPosY = newPlayerPosY;
        }        
    } else if (oldPlayerPosY < newPlayerPosY) {
        // Move down
        oldPlayerPosY += SCROLL_PLAYER_SPEED;
        if (oldPlayerPosY >= newPlayerPosY) {
            oldPlayerPosY = newPlayerPosY;
        }      
    }
	
	PLAYER.pos.x = oldPlayerPosX - SCROLL_CAM_X;
	PLAYER.pos.y = oldPlayerPosY - SCROLL_CAM_Y;
	
	if ((PLAYER.pos.x == transitioning_player_pos_x) && (PLAYER.pos.y == transitioning_player_pos_y)) {
        return TRUE;
    }
	return FALSE;
}
