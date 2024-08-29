#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_wifi.h"
#include "esp_http_server.h"
#include "driver/gpio.h"
#include <string.h>
#include <stdlib.h>

#define RELAY1_PIN GPIO_NUM_23  // Adjust these to your actual relay pins
#define RELAY2_PIN GPIO_NUM_22
#define RELAY3_PIN GPIO_NUM_21

static const char *TAG = "WATER_PUMP_CONTROLLER";

typedef struct {
    int relay_pin;
    int duration;
    int interval;
} relay_timer_config_t;

// Timer handler
void relay_timer_handler(void *arg) {
    relay_timer_config_t *config = (relay_timer_config_t *)arg;
    while (1) {
        gpio_set_level(config->relay_pin, 1);  // Turn on the relay
        vTaskDelay(config->duration / portTICK_PERIOD_MS);  // Keep it on for the specified duration
        gpio_set_level(config->relay_pin, 0);  // Turn off the relay
        vTaskDelay(config->interval / portTICK_PERIOD_MS);  // Wait for the interval before turning it on again
    }
}

// API handler to set up the timer
esp_err_t timer_handler(httpd_req_t *req) {
    char buf[100];
    int ret = httpd_req_recv(req, buf, sizeof(buf));
    if (ret <= 0) {
        httpd_resp_send_500(req);
        return ESP_FAIL;
    }

    buf[ret] = '\0';

    int relay = atoi(strtok(buf, "&"));
    int duration = atoi(strtok(NULL, "&"));
    int interval = atoi(strtok(NULL, "&"));

    int relay_pin;
    switch (relay) {
        case 1:
            relay_pin = RELAY1_PIN;
            break;
        case 2:
            relay_pin = RELAY2_PIN;
            break;
        case 3:
            relay_pin = RELAY3_PIN;
            break;
        default:
            httpd_resp_send_404(req);
            return ESP_FAIL;
    }

    relay_timer_config_t *config = malloc(sizeof(relay_timer_config_t));
    config->relay_pin = relay_pin;
    config->duration = duration;
    config->interval = interval;

    xTaskCreate(relay_timer_handler, "relay_timer_task", 2048, config, 5, NULL);

    httpd_resp_send(req, "Timer set", HTTPD_RESP_USE_STRLEN);
    ESP_LOGI(TAG, "Timer set for relay %d with duration %d ms and interval %d ms", relay, duration, interval);
    return ESP_OK;
}

// Handler to turn on the relay
esp_err_t on_handler(httpd_req_t *req) {
    int relay = atoi(req->uri + 4);  // Extract relay number from URI (e.g., /on1)

    int relay_pin;
    switch (relay) {
        case 1:
            relay_pin = RELAY1_PIN;
            break;
        case 2:
            relay_pin = RELAY2_PIN;
            break;
        case 3:
            relay_pin = RELAY3_PIN;
            break;
        default:
            httpd_resp_send_404(req);
            return ESP_FAIL;
    }

    gpio_set_level(relay_pin, 1);  // Turn on the relay
    httpd_resp_send(req, "Pump turned on", HTTPD_RESP_USE_STRLEN);
    ESP_LOGI(TAG, "Pump turned on for relay %d", relay);
    return ESP_OK;
}

// Handler to turn off the relay
esp_err_t off_handler(httpd_req_t *req) {
    int relay = atoi(req->uri + 5);  // Extract relay number from URI (e.g., /off1)

    int relay_pin;
    switch (relay) {
        case 1:
            relay_pin = RELAY1_PIN;
            break;
        case 2:
            relay_pin = RELAY2_PIN;
            break;
        case 3:
            relay_pin = RELAY3_PIN;
            break;
        default:
            httpd_resp_send_404(req);
            return ESP_FAIL;
    }

    gpio_set_level(relay_pin, 0);  // Turn off the relay
    httpd_resp_send(req, "Pump turned off", HTTPD_RESP_USE_STRLEN);
    ESP_LOGI(TAG, "Pump turned off for relay %d", relay);
    return ESP_OK;
}

// URI handler structures
httpd_uri_t on_uri = {
    .uri       = "/on*",  // Handles /on1, /on2, /on3
    .method    = HTTP_GET,
    .handler   = on_handler,
    .user_ctx  = NULL
};

httpd_uri_t off_uri = {
    .uri       = "/off*",  // Handles /off1, /off2, /off3
    .method    = HTTP_GET,
    .handler   = off_handler,
    .user_ctx  = NULL
};

httpd_uri_t timer_uri = {
    .uri       = "/timer",
    .method    = HTTP_POST,
    .handler   = timer_handler,
    .user_ctx  = NULL
};

// Function to start the web server
httpd_handle_t start_webserver(void) {
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    httpd_handle_t server = NULL;

    if (httpd_start(&server, &config) == ESP_OK) {
        httpd_register_uri_handler(server, &on_uri);
        httpd_register_uri_handler(server, &off_uri);
        httpd_register_uri_handler(server, &timer_uri);
    }

    return server;
}

// Wi-Fi event handler
static void wifi_event_handler(void* arg, esp_event_base_t event_base,
                               int32_t event_id, void* event_data) {
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        esp_wifi_connect();
        ESP_LOGI(TAG, "Retrying to connect to the AP...");
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        ESP_LOGI(TAG, "got ip:" IPSTR, IP2STR(&event->ip_info.ip));
    }
}

// Function to initialize Wi-Fi
void wifi_init(void) {
    tcpip_adapter_init();
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    esp_event_handler_instance_t instance_any_id;
    esp_event_handler_instance_t instance_got_ip;

    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                        ESP_EVENT_ANY_ID,
                                                        &wifi_event_handler,
                                                        NULL,
                                                        &instance_any_id));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
                                                        IP_EVENT_STA_GOT_IP,
                                                        &wifi_event_handler,
                                                        NULL,
                                                        &instance_got_ip));

    wifi_config_t wifi_config = {
        .sta = {
            .ssid = "DevMind5G",          // Replace with your Wi-Fi SSID
            .password = "E20444Delta$",  // Replace with your Wi-Fi password
        },
    };

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG, "wifi_init_sta finished.");
}

void app_main(void) {
    // Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    // Initialize Wi-Fi
    wifi_init();

    // Initialize the relay GPIOs
    gpio_pad_select_gpio(RELAY1_PIN);
    gpio_set_direction(RELAY1_PIN, GPIO_MODE_OUTPUT);
    gpio_set_level(RELAY1_PIN, 0);

    gpio_pad_select_gpio(RELAY2_PIN);
    gpio_set_direction(RELAY2_PIN, GPIO_MODE_OUTPUT);
    gpio_set_level(RELAY2_PIN, 0);

    gpio_pad_select_gpio(RELAY3_PIN);
    gpio_set_direction(RELAY3_PIN, GPIO_MODE_OUTPUT);
    gpio_set_level(RELAY3_PIN, 0);

    // Start the web server
    ESP_LOGI(TAG, "Starting web server");
    start_webserver();
}
