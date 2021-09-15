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
 * This file is for using an arduino to both control my garage door, and read data from sensors.
 */

#include "garage_door.h"
#include "door_control.h"
#include "motion_sensor.h"
#include "dht_sensor.h"
#include <DHT.h>
#include <EEPROM.h>

#define MOTION_PIN 3

#define POLL_ALL_COMMAND 0b10000000
#define DOOR_POSITION_COMMAND 0b10000001
#define CALIBRATE_OPEN 0b10000010
#define CALIBRATE_CLOSED 0b10000011
#define POLL_DOOR_COMMAND 0b10000100
#define POLL_DHT_COMMAND 0b10000101

#define DOOR_MAX_ADDRESS 0
#define DOOR_MIN_ADDRESS sizeof(int16_t)

#define DOOR_SHORT_POLL_TIME 1000

GarageDoor garage_door;

void process_command(DoorControl *door_control, uint8_t data);

void setup()
{  
  pinMode(MOTION_PIN, INPUT);
  Serial.begin(115200);
}

/**
 * Called when the next command is expected to be for the garage door.
 * If the command doesn't fit within 0-100 range then the command is treated as a new base command.
 * If no command arrives within a set time, it will be ignored and commands will be processed as normal.
 * @param garage_door a pointer to the garage door control struct
 */
void process_door_command(GarageDoor *garage_door)
{
  if (Serial.available()) {
    uint8_t data;
    data = Serial.read();
    if (data > 100) {
      // Door command is outside of range, process as command;
      process_command(garage_door, data);
    } else {
      garage_door->next_command = GENERIC_COMMAND;
      door_control_set_position(garage_door->door_control, data);
    }
  } else if (garage_door->next_command == GENERIC_COMMAND) {
    garage_door->command_timeout = millis() + 500;
    garage_door->next_command = DOOR_COMMAND;
  } else if (millis() > garage_door->command_timeout) {
    garage_door->next_command = GENERIC_COMMAND;
  }
}

/**
 * Processes a command read from serial.
 * @param garage_door a pointer to the garage door control struct
 * @param data the data read from serial
 */
void process_command(GarageDoor *garage_door, uint8_t data)
{
  int16_t position;
  switch (data)
  {
  case POLL_ALL_COMMAND:
    {
      uint8_t buffer[DHT_DATA_SIZE + DOOR_DATA_SIZE];
      uint8_t *position = buffer;
      position += dht_sensor_get(garage_door->dht, position);
      position += door_control_get(garage_door->door_control, position);
      Serial.write(buffer, DHT_DATA_SIZE + DOOR_DATA_SIZE);
    }
    break;
    case POLL_DHT_COMMAND:
    {
      uint8_t buffer[DHT_DATA_SIZE];
      dht_sensor_get(garage_door->dht, buffer);
      Serial.write(buffer, DHT_DATA_SIZE);
    }
    break;
    case POLL_DOOR_COMMAND:
    {
      uint8_t buffer[DOOR_DATA_SIZE];
      door_control_get(garage_door->door_control, buffer);
      Serial.write(buffer, DOOR_DATA_SIZE);
    }
    break;
  case DOOR_POSITION_COMMAND:
    process_door_command(garage_door);
    break;
  case CALIBRATE_OPEN:
    position = door_control_calibrate_max_position(garage_door->door_control);
    EEPROM.put(DOOR_MAX_ADDRESS, position);
    break;
  case CALIBRATE_CLOSED:
    position = door_control_calibrate_min_position(garage_door->door_control);
    EEPROM.put(DOOR_MIN_ADDRESS, position);
    break;
  default:
    break;
  }
}

/**
 * Main program loop
 */
void loop()
{
  GarageDoor *garage_door_ptr = &garage_door;

  // Reading values from the EEPROM for getting saved settings
  // Also sets ups components used for sub processes
  {
    // Getting the min/max calibration for the door control
    int16_t min, max;
    EEPROM.get( DOOR_MIN_ADDRESS, min );
    EEPROM.get( DOOR_MAX_ADDRESS, max );

    garage_door.door_control = malloc(sizeof(DoorControl));
    garage_door.dht = dht_sensor_initialize();

    door_control_initialize(garage_door.door_control, min, max);

    garage_door.next_command = GENERIC_COMMAND;
  }

  unsigned long short_timestamp = 0;

  while(true)
  {
    // Processing door controls
    if (door_control_execute(garage_door.door_control) && millis() > short_timestamp) {
      process_command(garage_door_ptr, POLL_DOOR_COMMAND);
      short_timestamp = millis() + DOOR_SHORT_POLL_TIME;
    }

    // Processing data from serial
    if (garage_door.next_command == GENERIC_COMMAND && Serial.available() > 0) {
      while (Serial.available() > 0) process_command(garage_door_ptr, Serial.read());
    } else if (garage_door.next_command == DOOR_COMMAND) {
      process_door_command(garage_door_ptr);
    }
  }
}
