/**
 * Application entry point.
 */

#include "nvs_flash.h"
#include "NTC.h"
#include "wifi_app.h"
#include "http_server.h"

int app_main(void)
{
	initializeNTC();

	// Initialize NVS
	esp_err_t ret = nvs_flash_init();
	if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
	{
		ESP_ERROR_CHECK(nvs_flash_erase());
		ret = nvs_flash_init();
	}
	ESP_ERROR_CHECK(ret);

	// Start Wifi
	wifi_app_start();

	initialize_sntp(); // Se conecta al servidor wen de donde se va a obtener la hora

	return 0;
}
