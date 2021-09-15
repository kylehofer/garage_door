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

#ifndef DHTSENSOR_H_
#define DHTSENSOR_H_

#include <DHT.h>
#include <stdint.h>

#define DHT_DATA_SIZE (sizeof(uint8_t) + sizeof(float) + sizeof(float))

DHT * dht_sensor_initialize();
uint8_t dht_sensor_get(DHT *dht, uint8_t *data);

#endif /* DHTSENSOR_H_ */