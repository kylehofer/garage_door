#!/usr/bin/env python
"""
Copyright (c) 2021 Kyle Hofer
Email: kylehofer@neurak.com.au
Web: https://neurak.com.au/

This file is part of GarageDoor.

GarageDoor is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

GarageDoor is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
"""
"""
This file is for creating a communication layer between MQTT and the Serial connection of the Arduino
"""

import paho.mqtt.client as mqtt
import serial
import time
import queue
import binascii
import struct
import logging
import atexit
import threading
from enum import Enum

logging.basicConfig(level=logging.INFO)

# Logger to log things
log = logging.getLogger("mqtt_arduino")

# I'm using queues to pass data between between threads
mqtt_queue = queue.Queue()
serial_queue = queue.Queue()

# Callback for when we connect to the mqtt broker
def on_connect(client, userdata, flags, rc):
    log.info("MQTT Connected with result code " + str(rc))
    # Subscribing in on_connect() means that if we lose the connection and
    # reconnect then subscriptions will be renewed.
    client.subscribe("workshop/door/command/#")

# Callback for when we recieve messages published to our subscribed topics.
def on_message(client, userdata, msg):
    log.debug("Payload recieved: " + msg.topic + " " + str(msg.payload))
    # Position command, we'll check it's an integer and within range
    if ("position" in msg.topic):
        if (msg.payload.isdigit()):
            position = int(msg.payload)
            if (position >= 0 and position <= 100):
                log.debug("Adding position command data to the queue: [129, " + str(position) + "]")
                serial_queue.put(bytearray([129, position]))            
            else:
                log.warning("Dumping position command as it is out of range [0-100]: " + str(position))
        else:
            log.warning("Dumping position command as it could not be converted to an integer: " + str(msg.payload))
    elif ("calibrate" in msg.topic):
        payload = msg.payload.decode("utf-8")
        if (payload == "open"):
            log.debug("Adding open calibration data to the queue: [130]")
            serial_queue.put(bytearray([130]))
        elif (payload == "closed"):
            log.debug("Adding closed calibration data to the queue: [131]")
            serial_queue.put(bytearray([131]))
        else:
            log.warning("Dumping calibration as command not recognized: " + payload)
    elif ("poll" in msg.topic):
        log.debug("Adding poll data to the queue: [128]")
        serial_queue.put(bytearray([128]))
    else:
        log.warning("Dumping command as it's not recognised: " + msg.topic + ", " + str(msg.payload))

# Used for processing data related to a door data
def door_data():
    data = serial_client.read(3)
    (position,) = struct.unpack('B', data[0:1])
    (state,) = struct.unpack('B', data[1:2])
    (result,) = struct.unpack('B', data[2:3])
    log.debug("Raw Data from Door Data: " + str(data))
    log.debug("After Processing - Position: " + str(position) + " State: " + str(state) + " Result: " + str(result))
    mqtt_client.publish("workshop/door/position", str(position))
    mqtt_client.publish("workshop/door/state", str(state))
    mqtt_client.publish("workshop/door/result", str(result))

# Used for processing data related to a dht sensor data
def dht_data():
    data = serial_client.read(8)
    (temperature,) = struct.unpack('f', data[0:4])
    (humidity,) = struct.unpack('f', data[4:8])
    log.debug("Raw Data from DHT Data: " + str(data))
    log.debug("After Processing - Temperature: " + str(temperature) + " Humidity: " + str(humidity))
    mqtt_client.publish("workshop/door/humidity", str(round(humidity, 2)))
    mqtt_client.publish("workshop/door/temperature", str(round(temperature, 2)))


# Map byte data to functions for processing
serial_options = {
    b'\x80' : door_data,
    b'\x81' : dht_data
}

mqtt_client = None
serial_client = None

# Polling function for polling the arduino for data
def poll_func():
    global serial_client
    threading.Timer(15, poll_func).start()
    if (serial_client and serial_client.isOpen()):
        log.debug("Adding poll data to the queue: [128]")
        serial_queue.put(bytearray([128]))

# On exit we make sure we close our Serial connection
def on_exit():
    log.info("Application Exiting")
    if (serial_client and serial_client.isOpen()):
        log.info("Closing Serial Connection")
        serial_client.close()

# Registering an on exit command
atexit.register(on_exit)

# Connecting to the arduino, will retry every 5 seconds if it fails
def serial_connect():
    serial_client = None
    while serial_client == None:
        try:
            log.info("Attempting Serial Connection")
            serial_client = serial.Serial('/dev/ttyACM0', 115200, serial.EIGHTBITS, serial.PARITY_NONE, serial.STOPBITS_ONE, timeout=3, write_timeout=3)
            serial_client.reset_input_buffer()
            log.info("Serial Connected")
        except serial.SerialException as e:
            serial_client = None
            log.warning("Serial Connection Failed. Retrying in 5 Seconds")
            time.sleep(5)
    return serial_client

# Main program loop
if __name__ == '__main__':
    # Set up our MQTT Client
    mqtt_client = mqtt.Client()
    mqtt_client.on_connect = on_connect
    mqtt_client.on_message = on_message
    log.info("Attempting MQTT Connection")
    mqtt_client.connect("192.168.1.20", 1883, 60)
    # Set up the mqtt client loop on a seperate thread
    mqtt_client.loop_start()
    serial_client = serial_connect()
    # Initializing our polling function
    threading.Timer(15, poll_func).start()
    # Loop which will handle serial data communication
    while 1:
        try:
            while serial_client.in_waiting > 0:
                # Data available to read
                data = serial_client.read()
                log.debug("Recieved data from Serial: " + str(data))
                if (data in serial_options):
                    serial_options[data]()
                
            while serial_queue.qsize() > 0:
                # Data available to write
                data = serial_queue.get_nowait()
                serial_client.write(data)
                serial_client.flush()
        except (serial.SerialException, IOError):
            # Close the connection on an error, and then attempt to reconnect
            log.warning("Serial communication experience an error, will close the connection and retry")
            if (serial_client and serial_client.isOpen()):
                serial_client.close()
            serial_client = serial_connect()
        time.sleep(0.01)
