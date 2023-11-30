#include "NTC.h"

QueueHandle_t uart_queue = 0;
QueueHandle_t ADC_lecture = 0;  // Se crea variable para almacenar cola de valores medidos por ADC
QueueHandle_t Temperaturas = 0; // Se crea variable para almacenar cola que contiene temperaturas tomadas en diferentes instantes de tiempo
QueueHandle_t dataLimitTemperature = 0;

bool initializeNTChandle = false;
int timerId = 0; // Identificación de cada timer, en este ejemplo no fue tan util ya que solamente hay uno

int Row1_Min = initRow1_Min;
int Row1_Max = initRow1_Max;
int Row2_Min = initRow2_Min;
int Row2_Max = initRow2_Max;
int Row3_Min = initRow3_Min;
int Row3_Max = initRow3_Max;

void initializeNTC(void)
{
    if (initializeNTChandle == false)
    {
        set_adc();         // Se configura el ADC
        create_task_NTC(); // Se crean las tareas
        initializeNTChandle = true;
    }
}

char *setLimitTemp(int row, int valueMin, int valueMax, int value_R, int value_G, int value_B)
{
    bool checkOK = true;
    char *msgReturn = malloc(32);
    char valueRow1Min[16], valueRow1Max[16], valueRow2Min[16], valueRow2Max[16], valueRow3Min[16], valueRow3Max[16];
    char stringOutput[32] = "COLD=";

    limitTempRBG dataLimitTempRBG = {
        .row = row,
        .valueMin = valueMin,
        .valueMax = valueMax,
        .value_R = value_R,
        .value_G = value_G,
        .value_B = value_B,
    };

    (row == 1 && ((valueMin > valueMax) || (valueMax > Row2_Min) || (valueMax > Row3_Min))) ? (checkOK = false) : NULL;
    (row == 2 && ((valueMin > valueMax) || (valueMin < Row1_Max) || (valueMax < valueMin) || (valueMax > Row3_Min))) ? (checkOK = false) : NULL;
    (row == 3 && ((valueMin > valueMax) || (valueMin < Row2_Max))) ? (checkOK = false) : NULL;

    /*printf("\nNewMIN:%d    NewMAX:%d   checkOK:%d\n", valueMin, valueMax, checkOK);
    printf("ROW 1---MIN:%d  MAX:%d\n", Row1_Min, Row1_Max);
    printf("ROW 2---MIN:%d  MAX:%d\n", Row2_Min, Row2_Max);
    printf("ROW 3---MIN:%d  MAX:%d\n\n", Row3_Min, Row3_Max);*/

    if (checkOK == true)
    {
        if (xQueueSend(dataLimitTemperature, &dataLimitTempRBG, portMAX_DELAY) == pdPASS)
        {
            msgReturn = "ValueOK";
        }
        else
        {
            ESP_LOGE(tag_task, "Error cargando datos a la cola 'dataLimitTemperature' ");
        }
    }
    else
    {

        itoa(Row1_Min, valueRow1Min, 10);
        strcat(stringOutput, valueRow1Min);
        strcat(stringOutput, "-");
        itoa(Row1_Max, valueRow1Max, 10);
        strcat(stringOutput, valueRow1Max);
        strcat(stringOutput, " ,MEDIUM=");
        itoa(Row2_Min, valueRow2Min, 10);
        strcat(stringOutput, valueRow2Min);
        strcat(stringOutput, "-");
        itoa(Row2_Max, valueRow2Max, 10);
        strcat(stringOutput, valueRow2Max);
        strcat(stringOutput, " ,WARM=");
        itoa(Row3_Min, valueRow3Min, 10);
        strcat(stringOutput, valueRow3Min);
        strcat(stringOutput, "-");
        itoa(Row3_Max, valueRow3Max, 10);
        strcat(stringOutput, valueRow3Max);
        printf("UNION= %s\n", stringOutput);
        // const char* output = stringOutput;
        msgReturn = stringOutput;
        printf("msgReturn= %s\n", msgReturn);
    }

    return msgReturn;
}

esp_err_t set_adc(void) // Función para configurar el puerto ADC
{
    adc1_config_channel_atten(NTC_PIN, ADC_ATTEN_DB_11); // Aquí se escoge el canar a utilizar y la atenuación que deseamos de acuerdo a nuestra señal
    adc1_config_width(ADC_WIDTH_BIT_12);                 // Aquí se escoge la resolución que deseamos para el ADC
    ESP_LOGI(tag_ADC, "ADC configured");
    return ESP_OK;
}

esp_err_t create_task_NTC(void) // Función en donde se crean y configuran las tareas
{

    ADC_lecture = xQueueCreate(15, sizeof(float)); // Se crea la cola de 10 espacios y valores de tipo flotante
    Temperaturas = xQueueCreate(1, sizeof(float)); // Se crea cola de 1 espacio con valores enteros
    dataLimitTemperature = xQueueCreate(1, sizeof(limitTempRBG));

    static uint8_t ucParameterToPass;
    TaskHandle_t xHandle = NULL;

    xTaskCreate(get_ADC,
                "get_ADC",
                SIZE_BUFFER_TASK,
                &ucParameterToPass,
                2,
                &xHandle);

    xTaskCreate(Promedio_temp,
                "Promedio_temp",
                SIZE_BUFFER_TASK,
                &ucParameterToPass,
                1,
                &xHandle);

    xTaskCreate(switchRGB,
                "switchRGB",
                SIZE_BUFFER_TASK,
                &ucParameterToPass,
                1,
                &xHandle);

    ESP_LOGI(tag_task, "Tasks created");

    return ESP_OK;
}

void switchRGB(void *pvParameters) // Tarea encargada de tomar 5 valores de lectura de temperatura y promediarlos
{
    limitTempRBG limitTemperature;
    float Temperatura = 0;

    int Row1_LedR = initRow1_LedR;
    int Row1_LedG = initRow1_LedG;
    int Row1_LedB = initRow1_LedB;
    int Row2_LedR = initRow2_LedR;
    int Row2_LedG = initRow2_LedG;
    int Row2_LedB = initRow2_LedB;
    int Row3_LedR = initRow3_LedR;
    int Row3_LedG = initRow3_LedG;
    int Row3_LedB = initRow3_LedB;

    while (1)
    {
        if (xQueueReceive(dataLimitTemperature, &limitTemperature, pdMS_TO_TICKS(50)) == pdPASS)
        {
            if (limitTemperature.row == 1)
            {
                Row1_Min = limitTemperature.valueMin;
                Row1_Max = limitTemperature.valueMax;
                Row1_LedR = limitTemperature.value_R;
                Row1_LedG = limitTemperature.value_G;
                Row1_LedB = limitTemperature.value_B;
                ESP_LOGW(tag_task, "COLD VALUES UPDATED");
            }
            if (limitTemperature.row == 2)
            {
                Row2_Min = limitTemperature.valueMin;
                Row2_Max = limitTemperature.valueMax;
                Row2_LedR = limitTemperature.value_R;
                Row2_LedG = limitTemperature.value_G;
                Row2_LedB = limitTemperature.value_B;
                ESP_LOGW(tag_task, "MEDIUM VALUES UPDATED");
            }
            if (limitTemperature.row == 3)
            {
                Row3_Min = limitTemperature.valueMin;
                Row3_Max = limitTemperature.valueMax;
                Row3_LedR = limitTemperature.value_R;
                Row3_LedG = limitTemperature.value_G;
                Row3_LedB = limitTemperature.value_B;
                ESP_LOGW(tag_task, "WARM VALUES UPDATED");
            }
        }

        xQueuePeek(Temperaturas, &Temperatura, pdMS_TO_TICKS(200));
        int countFalse = 0;
        (Temperatura > Row1_Min && Temperatura <= Row1_Max) ? updateRGB(Row1_LedR, Row1_LedG, Row1_LedB) : (countFalse += 1);
        (Temperatura > Row2_Min && Temperatura <= Row2_Max) ? updateRGB(Row2_LedR, Row2_LedG, Row2_LedB) : (countFalse += 1);
        (Temperatura > Row3_Min && Temperatura <= Row3_Max) ? updateRGB(Row3_LedR, Row3_LedG, Row3_LedB) : (countFalse += 1);
        (countFalse == 3) ? updateRGB(0, 0, 0) : NULL;

        /*printf("TEMPERATURA: %f     CONTADOR: %d\n", Temperatura, countFalse);
        printf("ROW 1---MIN:%d  MAX:%d   R:%d   G:%d    B:%d\n", Row1_Min, Row1_Max, Row1_LedR, Row1_LedG, Row1_LedB);
        printf("ROW 2---MIN:%d  MAX:%d   R:%d   G:%d    B:%d\n", Row2_Min, Row2_Max, Row2_LedR, Row2_LedG, Row2_LedB);
        printf("ROW 3---MIN:%d  MAX:%d   R:%d   G:%d    B:%d\n\n", Row3_Min, Row3_Max, Row3_LedR, Row3_LedG, Row3_LedB);*/

        vTaskDelay(pdMS_TO_TICKS(Delay_switchRGB));
    }
}

void Promedio_temp(void *pvParameters) // Tarea encargada de tomar 5 valores de lectura de temperatura y promediarlos
{

    while (1)
    {
        float receibedValue = 0, Sumatoria_temp = 0;

        // Ciclo for para leer 5 valores de tempertura y sumarlos
        for (size_t i = 0; i < 5; i++)
        {
            Sumatoria_temp += xQueueReceive(ADC_lecture, &receibedValue, pdMS_TO_TICKS(200)) ? receibedValue : printf("\x1b[31mError receiving value from queue\x1b[0m\n");
        }

        float Temperatura = Sumatoria_temp / 5; // Los valores sumados anteriormente se dividen en 5 para hallar su promedio y tener un valor que no sea tan volátil

        xQueueOverwrite(Temperaturas, &Temperatura); // Se guarda este valor en la cola "Temperaturas"

        vTaskDelay(pdMS_TO_TICKS(Delay_promedio));
    }
}

void get_ADC(void *pvParameters) // Tarea para leer temperatura y guardarla en una cola
{
    while (1)
    {
        int adc_val = 0;                                   // Variable para almacenar lectura
        adc_val = adc1_get_raw(NTC_PIN);                   // Funcion para leer el ADC. En este caso solamente nos pide el canal que deseamos leer
        float adc_value = (float)adc_val;                  // Se castea a valor flotante
        float Vol_NTC = (Vol_REF * adc_value) / 4095;      // Se calcula el voltaje que está cayendo en la NTC
        float R_NTC = R_FIXED / ((Vol_REF / Vol_NTC) - 1); // Se calcula la resistencia que tiene la NTC en ese momento. Esto se sacó despejando fórmulas del datasheet
        // float Temperatura_Kelvin = Beta/(log(R_NTC/R0_NTC)+(Beta/Temp0));
        float Temperatura_Kelvin = 1 / ((log(R_NTC / R0_NTC) / Beta) + (1 / Temp0)); // Se calcula la temperatura en grados Kelvin. También se hizo despejando
        float Temperatura_Celcius = Temperatura_Kelvin - 273.15;                     // Se convierte a grados Selcius

        // Guarda la el valor en la cola y en caso tal que no pueda, informa por medio de consola
        xQueueSend(ADC_lecture, &Temperatura_Celcius, pdMS_TO_TICKS(50)) ?: printf("\x1b[31mError writing in queue\x1b[0m\n");

        // ESP_LOGI(tag_ADC, "Lectura: %i, VOLTAJE: %f, R_NTC: %f, TEMPERATURA: %f", adc_val, Vol_NTC, R_NTC, Temperatura_Celcius);

        vTaskDelay(pdMS_TO_TICKS(Delay_promedio / 5));
    }
}