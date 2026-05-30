import time
import sys
import os

lib_path = '/usr/lib/python3/dist-packages'
sys.path.append(lib_path)

import pandas as pd
import locale
from datetime import datetime
from opcua import Server, ua
import logging
import asyncio
import paho.mqtt.client as mqtt

# MQTT Settings
BROKER_ADDRESS = "localhost"
PORT = 1883

device_name1 = "DGAD_1"
device_name2 = "DGAD_2"
device_name3 = "DGAD_3"

topic_name_1 = "Boot_Button_Status"
topic_name_2 = "Analog0-10V_Value"
topic_name_3 = "Analog4-20mA_Value"
topic_name_4 = "PT100_0-10V_Temp"
topic_name_5 = "PT100_4-20mA_Temp"


TOPIC1 = f"{device_name1}/{topic_name_1}"
TOPIC2 = f"{device_name1}/{topic_name_2}"
TOPIC3 = f"{device_name1}/{topic_name_3}"
TOPIC4 = f"{device_name1}/{topic_name_4}"
TOPIC5 = f"{device_name1}/{topic_name_5}"

TOPIC6 = f"{device_name2}/{topic_name_1}"
TOPIC7 = f"{device_name2}/{topic_name_2}"
TOPIC8 = f"{device_name2}/{topic_name_3}"
TOPIC9 = f"{device_name2}/{topic_name_4}"
TOPIC10 = f"{device_name2}/{topic_name_5}"

TOPIC11 = f"{device_name3}/{topic_name_1}"
TOPIC12 = f"{device_name3}/{topic_name_2}"
TOPIC13 = f"{device_name3}/{topic_name_3}"
TOPIC14 = f"{device_name3}/{topic_name_4}"
TOPIC15 = f"{device_name3}/{topic_name_5}"


def on_connect(client, userdata, flags, rc):
    print(f"Conectado com o codigo {rc}")
    client.subscribe(TOPIC1)
    client.subscribe(TOPIC2)
    client.subscribe(TOPIC3)
    client.subscribe(TOPIC4)
    client.subscribe(TOPIC5)
    client.subscribe(TOPIC6)
    client.subscribe(TOPIC7)
    client.subscribe(TOPIC8)
    client.subscribe(TOPIC9)
    client.subscribe(TOPIC10)
    client.subscribe(TOPIC11)
    client.subscribe(TOPIC12)
    client.subscribe(TOPIC13)
    client.subscribe(TOPIC14)
    client.subscribe(TOPIC15)


def on_message(client, userdata, msg):
    payload = msg.payload.decode()

    if msg.topic == TOPIC1:
        print(f"Recebido {msg.topic}: {payload}")
        var1.set_value(payload)

    elif msg.topic == TOPIC2:   
        print(f"Recebido {msg.topic}: {payload}")
        var2.set_value(payload)

    elif msg.topic == TOPIC3:   
        print(f"Recebido {msg.topic}: {payload}")
        var3.set_value(payload)

    elif msg.topic == TOPIC4:   
        print(f"Recebido {msg.topic}: {payload}")
        var4.set_value(payload)

    elif msg.topic == TOPIC5:   
        print(f"Recebido {msg.topic}: {payload}")
        var5.set_value(payload)

    elif msg.topic == TOPIC6:   
        print(f"Recebido {msg.topic}: {payload}")
        var6.set_value(payload)

    elif msg.topic == TOPIC7:   
        print(f"Recebido {msg.topic}: {payload}")
        var7.set_value(payload)

    elif msg.topic == TOPIC8:   
        print(f"Recebido {msg.topic}: {payload}")
        var8.set_value(payload)

    elif msg.topic == TOPIC9:   
        print(f"Recebido {msg.topic}: {payload}")
        var9.set_value(payload)

    elif msg.topic == TOPIC10:   
        print(f"Recebido {msg.topic}: {payload}")
        var10.set_value(payload)

    elif msg.topic == TOPIC11:   
        print(f"Recebido {msg.topic}: {payload}")
        var11.set_value(payload)  

    elif msg.topic == TOPIC12:   
        print(f"Recebido: {msg.topic}: {payload}")
        var12.set_value(payload)

    elif msg.topic == TOPIC13:   
        print(f"Recebido {msg.topic}: {payload}")
        var13.set_value(payload)

    elif msg.topic == TOPIC14:   
        print(f"Recebido {msg.topic}: {payload}")
        var14.set_value(payload)  

    elif msg.topic == TOPIC15:   
        print(f"Recebido: {msg.topic}: {payload}")
        var15.set_value(payload)       
   

server = Server()
server.set_endpoint("opc.tcp://192.168.137.179:4840/freeopcua/server/") 
server.set_server_name("DGAD FreeOPCUA Server")

uri = "http://examples.freeopcua.github.io"
idx = server.register_namespace(uri)

objects = server.get_objects_node()

        # Adiciona os objets e nodes OPCUA. A logica eh repetida para todos os dispositivos
obj_device_1 = objects.add_object(idx, device_name1)
var1 = obj_device_1.add_variable(idx, topic_name_1, 0) # Indica valor inicial
var1.set_writable()   # Permite a escrita na varivel
var2 = obj_device_1.add_variable(idx, topic_name_2, 0) 
var2.set_writable()    
var3= obj_device_1.add_variable(idx, topic_name_3, 0) 
var3.set_writable()    
var4 = obj_device_1.add_variable(idx, topic_name_4, 0) 
var4.set_writable()    
var5 = obj_device_1.add_variable(idx, topic_name_5, 0) 
var5.set_writable()    


obj_device_2 = objects.add_object(idx, device_name2)
var6 = obj_device_2.add_variable(idx, topic_name_1, 0) 
var6.set_writable()   
var7 = obj_device_2.add_variable(idx, topic_name_2, 0) 
var7.set_writable()    
var8= obj_device_2.add_variable(idx, topic_name_3, 0) 
var8.set_writable()   
var9 = obj_device_2.add_variable(idx, topic_name_4, 0) 
var9.set_writable()   
var10 = obj_device_2.add_variable(idx, topic_name_5, 0) 
var10.set_writable() 


obj_device_3 = objects.add_object(idx, device_name3)
var11 = obj_device_3.add_variable(idx, topic_name_1, 0) 
var11.set_writable()    
var12 = obj_device_3.add_variable(idx, topic_name_2, 0) 
var12.set_writable()    
var13= obj_device_3.add_variable(idx, topic_name_3, 0) 
var13.set_writable()    
var14 = obj_device_3.add_variable(idx, topic_name_4, 0) 
var14.set_writable()    
var15 = obj_device_3.add_variable(idx, topic_name_5, 0) 
var15.set_writable()  


server.start()
print(f"Server iniciado em {server.endpoint}")

      
client = mqtt.Client()
client.on_connect = on_connect
client.on_message = on_message

client.connect(BROKER_ADDRESS, PORT, 60)
client.loop_start() #Inicia o loop 

try:
    while True:

        time.sleep(.1)

except KeyboardInterrupt:
    print("Parando o programa...")
    client.loop
