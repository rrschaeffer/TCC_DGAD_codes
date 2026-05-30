import random
import paho.mqtt.client as mqtt
import time
from datetime import datetime
import csv

MQTT_BROKER = "192.168.137.179"
MQTT_PORT = 1883
DGAD_NAME = "DGAD_1"

PUBLISH_TOPIC_1 = "Boot_Button_Status"
PUBLISH_TOPIC_2 = "Analog0-10V_Value"
PUBLISH_TOPIC_3 = "Analog4-20mA_Value"
PUBLISH_TOPIC_4 = "PT100_0-10V_Temp"
PUBLISH_TOPIC_5 = "PT100_4-20mA_Temp"

PT100_1_TEMP_MIN  =  -50
PT100_1_TEMP_MAX  =  100
PT100_1_ADC_MIN   =  0
PT100_1_ADC_MAX   =  3200

PT100_2_TEMP_MIN  =  0
PT100_2_TEMP_MAX  =  200
PT100_2_ADC_MIN   =  640
PT100_2_ADC_MAX   =  3200

def on_connect(client, userdata, flags, rc):
    if rc == 0:
        print("Broker conectado!")
        client.subscribe(f"{DGAD_NAME}/{PUBLISH_TOPIC_1}")
        client.subscribe(f"{DGAD_NAME}/{PUBLISH_TOPIC_2}")
        client.subscribe(f"{DGAD_NAME}/{PUBLISH_TOPIC_3}")
        client.subscribe(f"{DGAD_NAME}/{PUBLISH_TOPIC_4}")
        client.subscribe(f"{DGAD_NAME}/{PUBLISH_TOPIC_5}")
    else:
        print(f"Houve um problema na conexão com o Broker! Código do erro: {rc}!")

def on_message(client, userdata, msg):

    msg_timestamp = datetime.now().strftime("%H:%M:%S.%f")[:-3]
    print(f"Tópico enviado: {msg.topic}, payload: {msg.payload.decode('utf-8')}. Timestamp: {msg_timestamp}")

    with open("MQTT_log.csv", mode='a', newline="", encoding='UTF-8') as csv_file:
        new_topic_csv = csv.writer(csv_file, delimiter=';')
        new_topic_csv.writerow([msg.topic, msg.payload.decode('utf-8') ,msg_timestamp])

def main():
    client = mqtt.Client()
    client.on_connect = on_connect
    client.on_message = on_message

    client.connect(MQTT_BROKER, MQTT_PORT)

    client.loop_start()

    try:
        while True:

            button_state = random.randint(0, 1)
            client.publish(f"{DGAD_NAME}/{PUBLISH_TOPIC_1}", button_state)

            adc_value1 = random.randint(PT100_1_ADC_MIN, PT100_1_ADC_MAX)
            client.publish(f"{DGAD_NAME}/{PUBLISH_TOPIC_2}", adc_value1)

            adc_value2 = random.randint(PT100_2_ADC_MIN, PT100_2_ADC_MAX)
            client.publish(f"{DGAD_NAME}/{PUBLISH_TOPIC_3}", adc_value2)

            temperature_conv = PT100_1_TEMP_MIN + (adc_value1-PT100_1_ADC_MIN)*((PT100_1_TEMP_MAX-PT100_1_TEMP_MIN)/(PT100_1_ADC_MAX-PT100_1_ADC_MIN))
            client.publish(f"{DGAD_NAME}/{PUBLISH_TOPIC_4}", f"{temperature_conv:.2f}")

            temperature_conv = PT100_2_TEMP_MIN + (adc_value2-PT100_2_ADC_MIN)*((PT100_2_TEMP_MAX-PT100_2_TEMP_MIN)/(PT100_2_ADC_MAX-PT100_2_ADC_MIN))
            client.publish(f"{DGAD_NAME}/{PUBLISH_TOPIC_5}", f"{temperature_conv:.2f}")

            time.sleep(1)

    except KeyboardInterrupt:
        print("Programa finalizado! Interrupção pelo usuário!")

    except Exception as erro:
        print(f"Um erro inesperado aconteceu! Erro: {erro}")

    finally:
        client.loop_stop()
        client.disconnect()




if __name__ == "__main__":
    main()