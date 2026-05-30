//CODIGO TESTE DGAD MQTT ESP-IDF
#include "stdio.h"
#include "stdint.h"
#include "stddef.h"
#include "string.h"

#include "esp_wifi.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "esp_log.h"

#include "freertos/task.h"
#include "freertos/projdefs.h"
#include "freertos/event_groups.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"

#include "lwip/err.h"
#include "lwip/sys.h"
#include "lwip/sockets.h"
#include "lwip/dns.h"
#include "lwip/netdb.h"

#include "esp_http_client.h"
#include "esp_netif.h"

#include "mqtt_client.h"

#include "driver/uart.h"
#include "driver/gpio.h"
#include "driver/adc.h"
#include "esp_adc_cal.h"

#define DELAY_MS            500
#define DELAY_MQTT          2000

#define ESP_WIFI_SSID      "Schaeffer"
#define ESP_WIFI_PASS      "12345678"
#define DGAD_NAME          "DGAD_1"

//#define MQTT_BROKER        "mqtt://broker.hivemq.com"
#define MQTT_BROKER        "mqtt://192.168.137.179"
#define MQTT_PORT           1883

#define PUBLISH_TOPIC_1     "Boot_Button_Status"
#define PUBLISH_TOPIC_2     "Digital_Input1_Status"
#define PUBLISH_TOPIC_3     "Analog0-10V_Value"
#define PUBLISH_TOPIC_4     "Analog0-10V_Conversion"


#define ESP_MAXIMUM_RETRY    5

#define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_WPA2_PSK

#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT      BIT1

//Definição de pinos e buffer da porta serial. No DGAD, utilizar UART0. 
#define UART_PORT           UART_NUM_0          // Usar UART0
#define TXD_PIN             GPIO_NUM_1          // Serial pino TX
#define RXD_PIN             GPIO_NUM_3          // Serial pino RX
#define ECHO_RTS            UART_PIN_NO_CHANGE  // Não altera configuração RTS
#define ECHO_CTS            UART_PIN_NO_CHANGE  // Não altera configuração CTS
#define BUF_SIZE            256                 // Tamanho do buffer para o RX da serial

//Definição dos pinos e características do ADC
#define ADC_UNIT_ID         ADC_UNIT_1          // DGAD só usa portas do ADC1
#define ADC_CHANNEL_ID      ADC_CHANNEL_4       // 0-10V_0   
#define ATTEN_DEF           ADC_ATTEN_DB_12     // Configuração da atenuação da leitura
#define VREF_DEF            1100                // Definição da tensão de referência para calibração do ADC para 1.1V
#define ADC_WIDTH_BIT       ADC_WIDTH_BIT_DEFAULT
#define ADC_V_OFFSET        0.25
#define ADC_VOLT_CONV       0.00368

//Definição dos pinos para os LEDs de exemplo (saídas) e do botão de BOOT (entrada)
#define LED0_PIN            GPIO_NUM_4          // Pino 26 IO4: LED0
#define LED1_PIN            GPIO_NUM_5          // Pino 29 IO5: LED1
#define BUTTON_PIN          GPIO_NUM_0          // Pino 25 IO0: botão BOOT
#define IO2_PIN             GPIO_NUM_13

//Declaração das variáveis utilizadas no exemplo
bool LED1_STATUS = false; 
char serial_mensage [BUF_SIZE]; 
int IO2_status = 0;
uint32_t adc_value, raw_adc_value; 

static const char *TAG = "wifi station";
static int s_retry_num = 0;
int isConnected = 0;

static char topic_1[30] = {""};
static char topic_2[30] = {""};
static char topic_3[30] = {""};
static char topic_4[30] = {""};

static EventGroupHandle_t s_wifi_event_group;
esp_mqtt_client_handle_t mqttClient;
static esp_adc_cal_characteristics_t adc_chars;

void DGAD_uart_config(void); // Função para configuração da porta serial
void DGAD_adc_config(void);  // Função para configuração da porta analógica
void DGAD_io_config(void);   // Função para configuração das portas digitais
void DGAD_led_blink(void);

void DGAD_digital_input_get_value(void);
void DGAD_analog_input_get_value(void);

void wifi_init_sta(void);
static void mqtt_start(void);
void mqtt_publish_task (void *pvParameters);

static void event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data);
static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data);

void app_main(void) {

    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
      ESP_ERROR_CHECK(nvs_flash_erase());
      ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    ESP_LOGI(TAG, "ESP_WIFI_MODE_STA");
    wifi_init_sta();

    sprintf(topic_1, "%s/%s", DGAD_NAME, PUBLISH_TOPIC_1);
    sprintf(topic_2, "%s/%s", DGAD_NAME, PUBLISH_TOPIC_2);
    sprintf(topic_3, "%s/%s", DGAD_NAME, PUBLISH_TOPIC_3);
    sprintf(topic_4, "%s/%s", DGAD_NAME, PUBLISH_TOPIC_4);

    if (isConnected) mqtt_start();

    DGAD_uart_config(); // Chama configuração da porta serial
    DGAD_adc_config();  
    DGAD_io_config();   // Chama configuração das portas digitais

    while(1) { // Loop infinito para polling 

        LED1_STATUS = !LED1_STATUS;
        gpio_set_level(LED1_PIN, LED1_STATUS);

        vTaskDelay(pdMS_TO_TICKS(DELAY_MS)); // Delay for 250ms
    }

}

void DGAD_uart_config(void){

    uart_config_t uart_config = {
        .baud_rate = 115200,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_APB,
    };

    ESP_ERROR_CHECK(uart_driver_install(UART_PORT, BUF_SIZE, 0, 0, NULL, 0));
    ESP_ERROR_CHECK(uart_param_config(UART_PORT, &uart_config));
    ESP_ERROR_CHECK(uart_set_pin(UART_PORT, TXD_PIN, RXD_PIN, ECHO_RTS, ECHO_CTS));
}

void DGAD_adc_config(void) {

    esp_adc_cal_characterize(ADC_UNIT_ID, ATTEN_DEF, ADC_WIDTH_BIT, VREF_DEF, &adc_chars); 
    adc1_config_width(ADC_WIDTH_BIT);
    adc1_config_channel_atten(ADC_CHANNEL_ID, ATTEN_DEF);

}

void DGAD_io_config(void){

    gpio_set_direction(LED0_PIN,   GPIO_MODE_OUTPUT);
    gpio_set_direction(LED1_PIN,   GPIO_MODE_OUTPUT);
    gpio_set_direction(BUTTON_PIN, GPIO_MODE_INPUT);
    gpio_set_direction(IO2_PIN,    GPIO_MODE_INPUT);

}

static void event_handler(void* arg, esp_event_base_t event_base,
                                int32_t event_id, void* event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        if (s_retry_num < ESP_MAXIMUM_RETRY) {
            esp_wifi_connect();
            s_retry_num++;
            ESP_LOGI(TAG, "retry to connect to the AP");
        } else {
            xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
        }
        ESP_LOGI(TAG,"connect to the AP fail");
        isConnected = 0;
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        ESP_LOGI(TAG, "got ip:" IPSTR, IP2STR(&event->ip_info.ip));
        s_retry_num = 0;
        isConnected = 1;
        xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
    }
}

void wifi_init_sta(void)
{
    s_wifi_event_group = xEventGroupCreate();

    ESP_ERROR_CHECK(esp_netif_init());

    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_t *sta_netif = esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    if (sta_netif){
        esp_netif_set_hostname(sta_netif, DGAD_NAME);
        ESP_LOGI("WiFi", "Hostname set to: %s", DGAD_NAME);
    }
        
    esp_event_handler_instance_t instance_any_id;
    esp_event_handler_instance_t instance_got_ip;
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                        ESP_EVENT_ANY_ID,
                                                        &event_handler,
                                                        NULL,
                                                        &instance_any_id));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
                                                        IP_EVENT_STA_GOT_IP,
                                                        &event_handler,
                                                        NULL,
                                                        &instance_got_ip));

    wifi_config_t wifi_config = {
        .sta = {
            .ssid = ESP_WIFI_SSID,
            .password = ESP_WIFI_PASS,
            /* Authmode threshold resets to WPA2 as default if password matches WPA2 standards (password len => 8).
             * If you want to connect the device to deprecated WEP/WPA networks, Please set the threshold value
             * to WIFI_AUTH_WEP/WIFI_AUTH_WPA_PSK and set the password with length and format matching to
             * WIFI_AUTH_WEP/WIFI_AUTH_WPA_PSK standards.
             */
            .threshold.authmode = ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD,
            .sae_pwe_h2e = WPA3_SAE_PWE_BOTH,
            .sae_h2e_identifier = "",
        },
    };
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA) );
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config) );
    ESP_ERROR_CHECK(esp_wifi_start() );

    ESP_LOGI(TAG, "wifi_init_sta finished.");

    /* Waiting until either the connection is established (WIFI_CONNECTED_BIT) or connection failed for the maximum
     * number of re-tries (WIFI_FAIL_BIT). The bits are set by event_handler() (see above) */
    EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group,
            WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
            pdFALSE,
            pdFALSE,
            portMAX_DELAY);

    /* xEventGroupWaitBits() returns the bits before the call returned, hence we can test which event actually
     * happened. */
    if (bits & WIFI_CONNECTED_BIT) {
        ESP_LOGI(TAG, "connected to ap SSID:%s password:%s",
                 ESP_WIFI_SSID, ESP_WIFI_PASS);
    } else if (bits & WIFI_FAIL_BIT) {
        ESP_LOGI(TAG, "Failed to connect to SSID:%s, password:%s",
                 ESP_WIFI_SSID, ESP_WIFI_PASS);
    } else {
        ESP_LOGE(TAG, "UNEXPECTED EVENT");
    }

}

static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
    ESP_LOGD(TAG, "Event dispatched from event loop base=%s, event_id=%" PRIi32, base, event_id);
    esp_mqtt_event_handle_t event = event_data;
    esp_mqtt_client_handle_t client = event->client;
    int msg_id;
    switch ((esp_mqtt_event_id_t)event_id) {
    case MQTT_EVENT_CONNECTED:
        ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");
      //  msg_id = esp_mqtt_client_subscribe(client, topic_1, 0);
      //  ESP_LOGI(TAG, "sent subscribe successful, msg_id=%d", msg_id);
	
		xTaskCreate(mqtt_publish_task, "mqtt_publish_task", 4096, NULL, 5, NULL);

        break;
        
    case MQTT_EVENT_DISCONNECTED:
        ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");
        break;

    case MQTT_EVENT_SUBSCRIBED:
        ESP_LOGI(TAG, "MQTT_EVENT_SUBSCRIBED, msg_id=%d, return code=0x%02x ", event->msg_id, (uint8_t)*event->data);
        break;
        
    case MQTT_EVENT_UNSUBSCRIBED:
        ESP_LOGI(TAG, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
        break;
        
    case MQTT_EVENT_PUBLISHED:
        ESP_LOGI(TAG, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
        break;
        
    case MQTT_EVENT_DATA:
        ESP_LOGI(TAG, "MQTT_EVENT_DATA");
        printf("TOPIC=%.*s\r\n", event->topic_len, event->topic);
        printf("DATA=%.*s\r\n", event->data_len, event->data);
        break;
        
    case MQTT_EVENT_ERROR:
        ESP_LOGI(TAG, "MQTT_EVENT_ERROR");
        if (event->error_handle->error_type == MQTT_ERROR_TYPE_TCP_TRANSPORT) {
            ESP_LOGI(TAG, "Last error code reported from esp-tls: 0x%x", event->error_handle->esp_tls_last_esp_err);
            ESP_LOGI(TAG, "Last tls stack error number: 0x%x", event->error_handle->esp_tls_stack_err);
            ESP_LOGI(TAG, "Last captured errno : %d (%s)",  event->error_handle->esp_transport_sock_errno,
                     strerror(event->error_handle->esp_transport_sock_errno));
        } else if (event->error_handle->error_type == MQTT_ERROR_TYPE_CONNECTION_REFUSED) {
            ESP_LOGI(TAG, "Connection refused error: 0x%x", event->error_handle->connect_return_code);
        } else {
            ESP_LOGW(TAG, "Unknown error type: 0x%x", event->error_handle->error_type);
        }
        break;
        
    default:
        ESP_LOGI(TAG, "Other event id:%d", event->event_id);
        break;
    }
}

static void mqtt_start(void)
{

    const esp_mqtt_client_config_t mqtt_cfg = {
        .broker = {
            .address.uri = MQTT_BROKER,
            .address.port = MQTT_PORT
        },
    };

    esp_mqtt_client_handle_t client = esp_mqtt_client_init(&mqtt_cfg);
    mqttClient = client;
    /* The last argument may be used to pass data to the event handler, in this example mqtt_event_handler */
    esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt_event_handler, NULL);
    esp_mqtt_client_start(client);
}

void mqtt_publish_task (void *pvParameters)
{
	char datatoSend[20];
    int msg_id, val;

	while (1)
    {
		//val = esp_random()%100;
        
		sprintf(datatoSend, "%d", gpio_get_level(BUTTON_PIN));
		msg_id = esp_mqtt_client_publish(mqttClient, topic_1, datatoSend, 0, 0, 0);
		if (msg_id == 0) ESP_LOGI(TAG, "Sent Data to topic %s: %s",topic_1, datatoSend);
		else ESP_LOGI(TAG, "Error msg_id:%d while sending data", msg_id);

        sprintf(datatoSend, "%d", gpio_get_level(IO2_PIN));
        msg_id = esp_mqtt_client_publish(mqttClient, topic_2, datatoSend, 0, 0, 0);
		if (msg_id == 0) ESP_LOGI(TAG, "Sent Data to topic %s: %s",topic_2, datatoSend);
		else ESP_LOGI(TAG, "Error msg_id:%d while sending data", msg_id);

        sprintf(datatoSend, "%d", adc1_get_raw(ADC_CHANNEL_ID));
        msg_id = esp_mqtt_client_publish(mqttClient, topic_3, datatoSend, 0, 0, 0);
		if (msg_id == 0) ESP_LOGI(TAG, "Sent Data to topic %s: %s",topic_3, datatoSend);
		else ESP_LOGI(TAG, "Error msg_id:%d while sending data", msg_id);

        sprintf(datatoSend, "%.1f", (esp_adc_cal_raw_to_voltage(adc1_get_raw(ADC_CHANNEL_ID), &adc_chars)*ADC_VOLT_CONV-ADC_V_OFFSET));
        msg_id = esp_mqtt_client_publish(mqttClient, topic_4, datatoSend, 0, 0, 0);
		if (msg_id == 0) ESP_LOGI(TAG, "Sent Data to topic %s: %s",topic_4, datatoSend);
		else ESP_LOGI(TAG, "Error msg_id:%d while sending data", msg_id);

		vTaskDelay(pdMS_TO_TICKS(DELAY_MQTT));
	}
}