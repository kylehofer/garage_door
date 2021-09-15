# door_control
Code for a custom garage door controller


[Unit]
Description=MQTT IO Publisher

[Service]
Restart=on-failure
RestartSec=5s

ExecStart=python3 -m mqtt_io /home/pi/mqtt/config.yml

[Install]
WantedBy=multi-user.target