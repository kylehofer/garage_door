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
 * This file is for reading data from a DHT sensor and putting that data into a buffer for serial transmission
 */

#include "dht_sensor.h"
#include <DHT.h>
#include <stdint.h>


#define DHTPIN 2
#define DHTTYPE DHT22
#define DHT_IDENTIFIER 0b10000001

// Union used for converting door data to a buffer for transmissions
typedef union {
    uint8_t buffer[DHT_DATA_SIZE];
    struct {
        uint8_t identifier;
        float temperature;
        float humidity;
    };    
} SensorToBuffer_u;

/**
 * Initializes the DHT class
 * @returns a pointer to the curstructed DHT class
 */
DHT * dht_sensor_initialize()
{
    DHT * dht = new DHT(DHTPIN, DHTTYPE);
    dht->begin();
    return dht;
}

/**
 * Reads data from the DHT Sensor and puts it into a buffer for sending over serial
 *
 * @param dht a pointer to a DHT class to read data from
 * @param data a pointer to the buffer to transfer data to
 * @return The number of bytes added to the buffer
 */
uint8_t dht_sensor_get(DHT *dht, uint8_t *data)
{
    SensorToBuffer_u sensor_data;
    sensor_data.identifier = DHT_IDENTIFIER;
    sensor_data.temperature = dht->readTemperature();
    sensor_data.humidity = dht->readHumidity();
    memcpy(data, sensor_data.buffer, DHT_DATA_SIZE);
    return DHT_DATA_SIZE;
}
