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
#include <DHT.h>

#define DHTPIN 2
#define DHTTYPE DHT22
#define MOTION_PIN 3

// DHT dht(DHTPIN, DHTTYPE);

void setup()
{  
  pinMode(MOTION_PIN, INPUT);
  Serial.begin(115200);
}

void loop()
{
  
  GarageDoor garage_door = door_control_initialize();
  
  while(true)
  {
    door_control_execute(&garage_door);
    if (Serial.available() > 0) {
      uint8_t data;
      char buffer[24];
      
      while (Serial.available() > 0)
      {
        data = Serial.read();
      }
    }
  }
}
