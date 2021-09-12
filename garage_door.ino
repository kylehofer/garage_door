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

#include <EEPROM.h>
#include "door_control.h"
#include <DHT.h>

#define DHTPIN 2
#define DHTTYPE DHT22
#define MOTION_PIN 3

#define POLL_COMMAND 0b10000000
#define DOOR_POSITION_COMMAND 0b10000001
#define CALIBRATE_OPEN 0b10000010
#define CALIBRATE_CLOSED 0b10000011


DHT dht(DHTPIN, DHTTYPE);

void setup()
{  
  pinMode(MOTION_PIN, INPUT);
  Serial.begin(115200);
}

void process_door_command(GarageDoor *garage_door)
{
  uint8_t data;
  unsigned long timeout = millis() + 500;
  while (Serial.available() == 0 && millis() > timeout);

  if (Serial.available()) {
    int8_t data;
    data = Serial.read();
    if (data < 0 || data > 100) {
      // Door command is outside of range, process as command;
      process_command(data);
    } else {
      door_control_set_position(garage_door, data);
    }
  }
}

void process_command(GarageDoor *garage_door, uint8_t data)
{
    switch (data)
    {
    case POLL_COMMAND:
      /* code */
      break;
    case DOOR_POSITION_COMMAND:
      process_door_command(garage_door);
      break;
    case CALIBRATE_OPEN:
      /* code */
      break;
    case CALIBRATE_CLOSED:
      /* code */
      break;
    default:
      break;
    }
  }
}

void loop()
{
  
  GarageDoor garage_door = door_control_initialize();
  GarageDoor *garage_door_ptr = &garage_door;
  
  while(true)
  {
    door_control_execute(garage_door_ptr);
    if (Serial.available() > 0) {    
      while (Serial.available() > 0) process_command(garage_door_ptr, Serial.read());
    }
  }
}
