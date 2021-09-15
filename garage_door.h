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
#ifndef GARAGEDOOR_H_
#define GARAGEDOOR_H_

#include "door_control.h"
#include "motion_sensor.h"
#include <DHT.h>
#include <stdint.h>

enum garage_command {
    GENERIC_COMMAND,
    DOOR_COMMAND
};


typedef struct _GarageDoor
{
    DoorControl *door_control;
    MotionSensor *motion_sensor;
    DHT *dht;
    garage_command next_command;
    unsigned long command_timeout;
} GarageDoor;

#endif /* GARAGEDOOR_H_ */