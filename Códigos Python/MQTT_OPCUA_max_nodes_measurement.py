import random
import asyncio
import sys
from aiomqtt import Client,MqttError
import csv

MQTT_BROKER = "192.168.137.179"
MQTT_PORT = 1883
DGAD_NAME = "DGAD_"

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

MINIMO_NODES = 1
MAXIMO_NODES = 100000
DELAY_PARA_ENVIO_MQTT = 1
DELAY_ENTRE_AUMENTO_NODES = 0.1

NUMERO_NODES_INICIAL=1
INCREMENTO_NODES_POR_CICLO = 1

ESTADO_BOTAO = 1
DADO_ADC1 = 1000
DADO_ADC2 = 1200

CONVERSAO_TEMPERATURA_1 = PT100_1_TEMP_MIN + (DADO_ADC1 - PT100_1_ADC_MIN) * (
        (PT100_1_TEMP_MAX - PT100_1_TEMP_MIN) / (PT100_1_ADC_MAX - PT100_1_ADC_MIN))

CONVERSAO_TEMPERATURA_2 = PT100_2_TEMP_MIN + (DADO_ADC2 - PT100_2_ADC_MIN) * (
        (PT100_2_TEMP_MAX - PT100_2_TEMP_MIN) / (PT100_2_ADC_MAX - PT100_2_ADC_MIN))

stats_mqtt = {"nodes conectados":0, "falhas de conexão":0, "mensagens enviadas": 0, "falhas de envio": 0}

async def node_connect(node_id: int):
    global stats_mqtt
    cliente_mqtt = f"{DGAD_NAME}{node_id}"


    topicos_mqtt = {

        f"{cliente_mqtt}/{PUBLISH_TOPIC_1}": 0 ,
        f"{cliente_mqtt}/{PUBLISH_TOPIC_2}": 0,
        f"{cliente_mqtt}/{PUBLISH_TOPIC_3}": 0,
        f"{cliente_mqtt}/{PUBLISH_TOPIC_4}": 0.00,
        f"{cliente_mqtt}/{PUBLISH_TOPIC_5}": 0.00
    }

    try:

        async with Client(hostname=MQTT_BROKER, port=MQTT_PORT, identifier=cliente_mqtt, timeout=100) as cliente:
            stats_mqtt["nodes conectados"] += 1

            while True:

                topicos_mqtt[f"{cliente_mqtt}/{PUBLISH_TOPIC_1}"] = ESTADO_BOTAO
                topicos_mqtt[f"{cliente_mqtt}/{PUBLISH_TOPIC_2}"] = DADO_ADC1
                topicos_mqtt[f"{cliente_mqtt}/{PUBLISH_TOPIC_3}"] = DADO_ADC2
                topicos_mqtt[f"{cliente_mqtt}/{PUBLISH_TOPIC_4}"] = CONVERSAO_TEMPERATURA_1
                topicos_mqtt[f"{cliente_mqtt}/{PUBLISH_TOPIC_5}"] = CONVERSAO_TEMPERATURA_2

                for topico in topicos_mqtt:

                    try:
                        await cliente.publish(topico, payload=topicos_mqtt[topico])
                        stats_mqtt["mensagens enviadas"] += 1

                    except MqttError:
                        stats_mqtt["falhas de envio"] += 1

                    finally:
                        pass

                await asyncio.sleep(DELAY_PARA_ENVIO_MQTT)

    except MqttError:
        stats_mqtt["falhas de conexão"] += 1

    finally:

        if cliente_mqtt in locals() and "client" in locals():
            stats_mqtt["nodes conectados"] = max(0, stats_mqtt["nodes conectados"] - 1)


async def monitor_status_mqtt_task():
    """Tarefa paralela que exibe o status do teste no terminal em tempo real."""
    print("\n[INFO] Iniciando monitoramento. Pressione Ctrl+C para encerrar.")
    print(f"{'Tempo (s)':<10} | {'Nodes Ativos':<14} | {'Falhas Conexão':<16} | {'Mensagens enviadas':<15} | {'falhas envio':<12}")
    print("-" * 75)


    tempo_decorrido = 0

    while True:
        await asyncio.sleep(1)
        tempo_decorrido += 1
        print(f"{tempo_decorrido: <10} | {stats_mqtt["nodes conectados"]:<14} | {stats_mqtt["falhas de conexão"]:<16} "
              f"|{stats_mqtt["mensagens enviadas"]:<15} | {stats_mqtt["falhas de envio"]:<12}")

        with open("MQTT_stats.csv", mode='a', newline="", encoding='UTF-8') as csv_file:
            new_topic_csv = csv.writer(csv_file, delimiter=';')
            new_topic_csv.writerow([tempo_decorrido] + list(stats_mqtt.values()))



        sys.stdout.flush()

async def main():
    asyncio.create_task(monitor_status_mqtt_task())

    lista_tarefas = []
    numero_nodes_atual = NUMERO_NODES_INICIAL
    print(f"[AUMENTO DE NODES] adicionando 1 node a cada {DELAY_ENTRE_AUMENTO_NODES} segundos.")

    try:
        while numero_nodes_atual <= MAXIMO_NODES:

            numero_nodes_atual +=INCREMENTO_NODES_POR_CICLO
            tarefa = asyncio.create_task(node_connect(numero_nodes_atual))
            lista_tarefas.append(tarefa)

            if numero_nodes_atual >= MAXIMO_NODES:
                break

            await asyncio.sleep(DELAY_ENTRE_AUMENTO_NODES)

        print("LIMITE MÁXIMO DE NODES ATINGIDO!")
        await asyncio.gather(*lista_tarefas, return_exceptions=True)

    except asyncio.CancelledError:
        print("Encerrando o teste...")


if sys.platform == "win32":
    asyncio.set_event_loop_policy(asyncio.WindowsSelectorEventLoopPolicy())

try:
    asyncio.run(main())
except KeyboardInterrupt:
    print("\nTeste abortado pelo usuário.")
