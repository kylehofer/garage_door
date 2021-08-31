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

#include "door_control.h"
#include "Arduino.h"

// Pin Configurations
#define RELAY_PIN 9
#define POTENTIOMETER_PIN A0

// Timers
#define RELAY_DELAY 100
#define DOOR_SAMPLE_RATE 35
#define DOOR_COMMAND_DELAY 1250

// Thresholds
#define DELTA_THRESHOLD 1
#define DOOR_THRESHOLD 35
#define DOOR_MIN DOOR_THRESHOLD
#define DOOR_MAX 978 - DOOR_THRESHOLD

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Initializes the door control component and sets up pins for controlling the relay.
 * @returns a garage door control struct
 */
GarageDoor door_control_initialize()
{
    pinMode(RELAY_PIN, OUTPUT);

    GarageDoor garage_door = (GarageDoor) {
        .current_state = IDLE,
        .desired_state = IDLE,
        .result = NONE,
        .current_position = analogRead(POTENTIOMETER_PIN),
        .desired_position = analogRead(POTENTIOMETER_PIN),
        .mean_position_delta = 0,
        .relay_output = 0,
        .time_stamp = 0,
        .command_time = 0,
        .relay_time = 0,
        .stablize_count = 0,
        .hold_transition = 0
    };

    return garage_door;
}

/**
 * Sets the targeted position for the garage door.
 *
 * @param garage_door a pointer to the garage door control struct
 * @param position 0-1024 value for the position of the garage door
 */
void door_control_set_position(GarageDoor *garage_door, int16_t position)
{
   garage_door->desired_position = position;
   garage_door->desired_state = ACTIVE;
   garage_door->result = NONE;
}

/**
 * Gets the current position for the garage door.
 * @param garage_door a pointer to the garage door control struct
 * @returns
 */
int door_control_execute(GarageDoor *garage_door)
{
    if (garage_door->relay_output == HIGH && garage_door->relay_time < millis()) {
        digitalWrite(RELAY_PIN, garage_door->relay_output = LOW);
    }

    if (garage_door->time_stamp > millis()) {
        return -1;
    } else {
        garage_door->time_stamp = millis() + DOOR_SAMPLE_RATE;
    }
    

    int16_t potentiometer_position = analogRead(POTENTIOMETER_PIN);
    int16_t difference = garage_door->current_position - potentiometer_position;
    int8_t is_moving = garage_door->mean_position_delta > DELTA_THRESHOLD || garage_door->mean_position_delta < -DELTA_THRESHOLD;

    garage_door->mean_position_delta = (garage_door->mean_position_delta + difference) >> 1;

    switch (garage_door->current_state)
    {
    case START:
        if (is_moving) {
            if (--garage_door->stablize_count == 0) {
                garage_door->desired_state = 
                    (garage_door->mean_position_delta < 0 && garage_door->current_position < garage_door->desired_position) ||
                    (garage_door->mean_position_delta > 0 && garage_door->current_position > garage_door->desired_position)
                    ? MONITOR : ACTIVE;
            }            
        } else {
            garage_door->stablize_count = 4;
            if (garage_door->command_time < millis()) garage_door->desired_state = ACTIVE;
        }
        break;
    case STOP:
        if (!is_moving) {
            if (--garage_door->stablize_count == 0) {
                 garage_door->desired_state = ACTIVE;
            }           
        } else {
            garage_door->stablize_count = 8;
        }
        break;
    case MONITOR:
        if (garage_door->current_position > (garage_door->desired_position - DOOR_THRESHOLD) &&
            garage_door->current_position < (garage_door->desired_position + DOOR_THRESHOLD))
            {
                garage_door->desired_state = garage_door->desired_position > DOOR_MIN && garage_door->desired_position < DOOR_MAX ? STOP : IDLE;
                garage_door->result = SUCCESS;
            } else if ((garage_door->mean_position_delta > 0 && garage_door->current_position < garage_door->desired_position) ||
                (garage_door->mean_position_delta < 0 && garage_door->current_position > garage_door->desired_position)) {
                garage_door->desired_state = IDLE;
                garage_door->result = FAIL;
            }
        break;
    case ACTIVE:
        garage_door->desired_state = 
            garage_door->current_position > (garage_door->desired_position - DOOR_THRESHOLD) &&
            garage_door->current_position < (garage_door->desired_position + DOOR_THRESHOLD)
            ? IDLE : START;
        break;
    default:
        break;
    }

    garage_door->current_position = potentiometer_position;

    if (garage_door->desired_state != garage_door->current_state)
    {
        if (garage_door->desired_state == START || garage_door->desired_state == STOP) {
            digitalWrite(RELAY_PIN, garage_door->relay_output = HIGH);
            garage_door->command_time = millis() + DOOR_COMMAND_DELAY;
            garage_door->relay_time = millis() + RELAY_DELAY;
        }
        garage_door->current_state = garage_door->desired_state;
        return 1;
    }

    return is_moving;
}

#ifdef __cplusplus
}
#endif