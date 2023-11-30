#include "esp_http_server.h"
#include "esp_log.h"
#include "esp_http_client.h"

#include "http_server.h"
#include "tasks_common.h"
#include "wifi_app.h"

#include <time.h>
#include <stdio.h>

// Tag utilizado para mensajes en la consola serial de ESP
static const char TAG[] = "http_server";

// Handle del servidor HTTP
static httpd_handle_t http_server_handle = NULL;

// URL del servidor remoto que proporciona la hora
#define TIME_SERVER_URL "http://cronos.unad.edu.co"  // Reemplaza con la URL real del servidor

// Función para obtener la hora desde el servidor remoto
static void get_time_from_server(char *time_buffer, size_t buffer_size) {
    esp_http_client_config_t config = {
        .url = TIME_SERVER_URL,
    };

    esp_http_client_handle_t client = esp_http_client_init(&config);
    esp_err_t err = esp_http_client_perform(client);

    if (err == ESP_OK) {
        int content_length = esp_http_client_get_content_length(client);
        char *response_data = malloc(content_length + 1);
        esp_http_client_read(client, response_data, content_length);
        response_data[content_length] = '\0';

        strncpy(time_buffer, response_data, buffer_size);

        free(response_data);
    } else {
        ESP_LOGE(TAG, "HTTP request failed");
        snprintf(time_buffer, buffer_size, "N/A");
    }

    esp_http_client_cleanup(client);
}

// Handler para la página web que muestra la hora del servidor remoto
static esp_err_t http_server_index_html_handler(httpd_req_t *req) {
    ESP_LOGI(TAG, "index.html requested");

    httpd_resp_set_type(req, "text/html");

    char time_buffer[64];  // Tamaño suficiente para almacenar la hora
    get_time_from_server(time_buffer, sizeof(time_buffer));

    char html_response[128];
    snprintf(html_response, sizeof(html_response), "<html><body><h1>Hora del servidor: %s</h1></body></html>", time_buffer);
    httpd_resp_sendstr(req, html_response);

    return ESP_OK;
}

// Función para configurar el servidor HTTP
static httpd_handle_t http_server_configure(void) {
    // Genera la configuración por defecto
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();

    // Inicia el servidor HTTP
    if (httpd_start(&http_server_handle, &config) == ESP_OK) {
        ESP_LOGI(TAG, "http_server_configure: Registrando manejadores de URI");

        // Registra el manejador para index.html
        httpd_uri_t index_html = {
            .uri = "/",
            .method = HTTP_GET,
            .handler = http_server_index_html_handler,
            .user_ctx = NULL
        };
        httpd_register_uri_handler(http_server_handle, &index_html);

        return http_server_handle;
    }

    return NULL;
}

// Función para obtener la hora desde el servidor remoto
void http_server_get_time_task(void *pvParameter) {
    while (1) {
        char time_buffer[64];  // Tamaño suficiente para almacenar la hora
        get_time_from_server(time_buffer, sizeof(time_buffer));

        // Espera 60 segundos antes de realizar la próxima solicitud
        vTaskDelay(60000 / portTICK_PERIOD_MS);
    }
}

// Iniciar el servidor HTTP
void http_server_start(void) {
    if (http_server_handle == NULL) {
        http_server_handle = http_server_configure();
    }

    // Iniciar tarea para obtener la hora desde el servidor remoto
    xTaskCreate(&http_server_get_time_task, "get_time_task", 4096, NULL, 5, NULL);
}

// Detener el servidor HTTP
void http_server_stop(void) {
    if (http_server_handle) {
        httpd_stop(http_server_handle);
        ESP_LOGI(TAG, "http_server_stop: Deteniendo el servidor HTTP");
        http_server_handle = NULL;
    }
}

void get_current_time(char *time_buffer, size_t buffer_size) {
    time_t now;
    struct tm timeinfo;
    time(&now);
    localtime_r(&now, &timeinfo);
    
    snprintf(time_buffer, buffer_size, "%02d:%02d:%02d", timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
}

