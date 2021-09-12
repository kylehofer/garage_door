import paho.mqtt.client as mqtt # Import the MQTT library
import serial
import time # The time library is useful for delays
import Queue

 
Garage/Door/Position

mqtt_queue = Queue.Queue()
serial_queue = Queue.Queue()

# Our "on message" event

# The callback for when the client receives a CONNACK response from the server.
def on_connect(client, userdata, flags, rc):
    print("Connected with result code "+str(rc))
    # Subscribing in on_connect() means that if we lose the connection and
    # reconnect then subscriptions will be renewed.
    client.subscribe("Garage/Door/#")

# The callback for when a PUBLISH message is received from the server.
def on_message(client, userdata, msg):
    print(msg.topic+" "+str(msg.payload))

mqtt_client = mqtt.Client()
mqtt_client.on_connect = on_connect
mqtt_client.on_message = on_message

mqtt_client.connect("192.168.1.20", 1883, 60)

mqtt_client.loop_start()





if __name__ == '__main__':
    serial_client = serial.Serial('/dev/ttyACM0', 9600, timeout=1)
    serial_client.flush()
    while 1:
        while ser.in_waiting > 0:
            line = ser.readline().decode('utf-8').rstrip()
            print(line)

# Main program loop

while(1):

ourClient.publish("AC_unit", "on") # Publish message to MQTT broker

time.sleep(1) # Sleep for a second



#!/usr/bin/env python3
import serial
import time
user_input=1

if __name__ == '__main__':
    ser = serial.Serial('/dev/ttyACM0', 9600, timeout=1)
    ser.flush()
    while True:
#        while ser.in_waiting > 0:
#            line = ser.readline().decode('utf-8').rstrip()
#            print(line)
        # get keyboard input
        user_input = input(">> ")
        if user_input == 'exit':
            ser.close()
            exit()
        else:
            # send the character to the device
            # (note that I happend a \r\n carriage return and line feed to the characters - this is requested by my device)
            ser.write(user_input.encode())

#!/usr/bin/env python3

user_input=1
