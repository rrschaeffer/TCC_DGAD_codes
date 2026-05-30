import paho.mqtt.client as mqtt
import locale
import time
from datetime import datetime

# MQTT Settings
#BROKER_ADDRESS = "192.168.1.59" 
BROKER_ADDRESS = "localhost"
PORT = 1883

TOPIC1 = "DGAD_1/Boot_Button_Status"
TOPIC2 = "DGAD_1/Digital_Input1_Status"
TOPIC3 = "DGAD_1/Analog0-10V_Value"
TOPIC4 = "DGAD_1/Analog0-10V_Conversion"


def on_connect(client, userdata, flags, rc):
    print(f"Conectado com o retorno {rc}")
    client.subscribe(TOPIC1)
    client.subscribe(TOPIC2)
    client.subscribe(TOPIC3)
    client.subscribe(TOPIC4)

def on_message(client, userdata, msg):
    payload = msg.payload.decode()

    if msg.topic == TOPIC1:
        print(f"Received on {msg.topic}: {payload}")

    elif msg.topic == TOPIC2:   
        print(f"Recebido por {msg.topic}: {payload}")

    elif msg.topic == TOPIC3:   
        print(f"Recebido por {msg.topic}: {payload}")
    
    elif msg.topic == TOPIC4:   
        print(f"Recebido por {msg.topic}: {payload}")
    
client = mqtt.Client()
client.on_connect = on_connect
client.on_message = on_message

client.connect(BROKER_ADDRESS, PORT, 60)
client.loop_start()

try:
    while True:
#      	print("\n")
        time.sleep(.5)

except KeyboardInterrupt:
    print("Encerrando o programa.")
    client.loop
