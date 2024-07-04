#pragma bank 255

#include <gbdk/platform.h>
#include "system.h"
#include "vm.h"
#include "bankdata.h"
#include "data_manager.h"
#include "camera.h"
#include "actor.h"
#include "scroll.h"
#include "scene_transition.h"

void set_neighbour_scene(SCRIPT_CTX * THIS) OLDCALL BANKED {
	uint8_t scene_bank = *(uint8_t *) VM_REF_TO_PTR(FN_ARG0);
	const scene_t * scene = *(scene_t **) VM_REF_TO_PTR(FN_ARG1);
	uint8_t direction = *(uint8_t *)VM_REF_TO_PTR(FN_ARG2);
	uint8_t rounding = *(uint8_t *)VM_REF_TO_PTR(FN_ARG3);
	enable_transition_to_scene();	
	if (rounding){
		round_position_flags |= direction;
	}
	if (direction == DIRECTION_UP){
		up_scene.bank = scene_bank;
		up_scene.ptr = (void *)scene;
	} else if (direction == DIRECTION_RIGHT){
		right_scene.bank = scene_bank;
		right_scene.ptr = (void *)scene;
	} else if (direction == DIRECTION_DOWN){
		down_scene.bank = scene_bank;
		down_scene.ptr = (void *)scene;
	} else if (direction == DIRECTION_LEFT){
		left_scene.bank = scene_bank;
		left_scene.ptr = (void *)scene;
	}
}
