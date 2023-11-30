#ifndef CONFIG_NTC
#define CONFIG_NTC

#include <stdio.h>
#include "driver/adc.h" 
#include "esp_log.h"    
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h" 
#include <stdlib.h>
#include <math.h>          
#include <string.h>       
#include "freertos/task.h"
#include "rgb_led.h"

#define NTC_PIN ADC1_CHANNEL_6     // Canal ocupado para la lectura ADC
#define R_FIXED 100000            // Valor de resistencia adicional que se pone para hacer el valor de tensión con la NTC
#define R0_NTC 100000             // Valor de NTC a 25°C
#define Beta 4190                 // Factor Beta de la NTC
#define Temp0 298.15              // Valor de temperatura a temperatura ambiente, en °Kelvin. (25°C+273.15=298.15°K)
#define Vol_REF 3.3               // Voltaje aplicado al divisor de tensión
#define SIZE_BUFFER_TASK 1024 * 2 // valor de espacio de memoria para las tareas (si se pone un valor muy pequeño se  reinicia el micro)
#define Delay_promedio 500
#define Delay_switchRGB 1000
#define initRow1_Min 15
#define initRow1_Max 25
#define initRow1_LedR 0
#define initRow1_LedG 0
#define initRow1_LedB 100
#define initRow2_Min 25
#define initRow2_Max 35
#define initRow2_LedR 0
#define initRow2_LedG 100
#define initRow2_LedB 0
#define initRow3_Min 35
#define initRow3_Max 45
#define initRow3_LedR 100
#define initRow3_LedG 0
#define initRow3_LedB 0

static const char *tag_task = "Task"; // Variable utilzada para etiquetar con la palabra "Task" el mensaje enviado por medio de ESP_LOG()

// Prototipado de funciones
void initializeNTC(void);
char* setLimitTemp(int row, int valueMin, int valueMax, int value_R, int value_G, int value_B);
esp_err_t create_task_NTC(void);
void get_ADC(void *pvParameters);
void Promedio_temp(void *pvParameters);
void switchRGB(void *pvParameters);

static const char *tag_ADC = "ADC"; // Variable utilzada para etiquetar con la palabra "ADC" el mensaje enviado por medio de ESP_LOG()

// Prototipado de funciones
esp_err_t set_adc(void);

typedef struct
{
    int row;
    int valueMin;
    int valueMax;
    int value_R;
    int value_G;
    int value_B;

} limitTempRBG;

#endif