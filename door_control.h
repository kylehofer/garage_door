/*
 * Copyright (c) 2021 Kyle Hofer
 * Email: kylehofer@neurak.com.au
 * Web: https://neurak.com.au/
 *
 * This file is part of GarageDoor.
 *
 * GarageDoor is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * GarageDoor is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
/*
 * This file is for controlling my garage door. This is done by closing a relay to activate the door.
 * A potentiometer is used to read the position of the door.
 */
#ifndef DOORCONTROL_H_
#define DOORCONTROL_H_

#include "Arduino.h"

enum door_state {
    ACTIVE,
    MONITOR,
    START,
    STOP,
    IDLE
};

enum door_result {
    NONE,
    SUCCESS,
    FAIL
};


typedef struct _GarageDoor
{
    enum door_state current_state;
    enum door_state desired_state;
    enum door_result result;
    int16_t current_position;
    int16_t desired_position;
    int8_t mean_position_delta;
    uint8_t relay_output;
    unsigned long time_stamp;
    unsigned long command_time;
    unsigned long relay_time;
    uint8_t stablize_count;
    uint8_t hold_transition;
} GarageDoor;

#ifdef __cplusplus
extern "C" {
#endif

void door_control_set_position(GarageDoor *garage_door, int16_t position);
int door_control_execute(GarageDoor *garage_door);
GarageDoor door_control_initialize();

#ifdef __cplusplus
}
#endif

#endif /* DOORCONTROL_H_ */