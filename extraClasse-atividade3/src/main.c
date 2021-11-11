/*
Criar um projeto com FreeRTOS com 3 Tasks com uso de filas para comunicação entre elas:
    Task 1: leitura do valor de um sensor analógico
    Task 2: exibição do valor no display 
    Task 3: exibição do valor no ESPlog
*/
#include <stdio.h>
#include "driver/gpio.h"
#include <driver/adc.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_system.h"
#include "esp_log.h"

TaskHandle_t xTask1Handle, xTask2Handle, xTask3Handle;
void vTask_ADC(void *pvParameters); 
void vTask_display(void *pvParameters); 
void vTask_console(void *pvParameters); 

// Queue structure for task synchronization and communication
QueueHandle_t xFila;

/**
 * GPIOs configuration and initialize 
 */
void USER_GPIO_INIT(void){
    //ADC Channel initialize
    adc1_config_width(ADC_WIDTH_BIT_12);
    adc1_config_channel_atten(ADC1_CHANNEL_0, ADC_ATTEN_DB_11);
};

/**
 * Task 1: leitura do valor de um sensor analógico
 */
void vTask_ADC(void *pvParameters){
    (void) pvParameters;
  
    int read_raw;
    while(1){
        esp_err_t adcReadResult = adc2_get_raw( ADC1_CHANNEL_0, ADC_ATTEN_DB_11, &read_raw);
        if (adcReadResult != ESP_OK) {
            printf("*** ADC Channel read fault ***\n");
        }

        uint32_t adc = 0;
        for (int i = 0; i < 100; i++){
            adc += adc1_get_raw(ADC1_CHANNEL_0);
            ets_delay_us(30);
        }
        adc /= 100;

        printf("*** Valor enviado: %u\n", adc);
        xQueueSend(xFila, &adc, portMAX_DELAY) ;
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
};


/**
 * Task 2: exibição do valor no display 
 */
void vTask_display(void *pvParameters){
    (void) pvParameters;
    int valor_recebido = 0;

    while(1){
        if(xQueuePeek(xFila, &valor_recebido, portMAX_DELAY)){
            ESP_LOGI("vTask_display", "Item impresso: %u", valor_recebido);
        }
    }
};

/**
 * Task 3: exibição do valor no ESPlog
 */
void vTask_console(void *pvParameters){
    (void) pvParameters;
    int valor_recebido = 0;

    while(1){
        if(xQueueReceive(xFila, &valor_recebido, portMAX_DELAY)){
            ESP_LOGI("vTask_console", "Item impresso: %u", valor_recebido);
        }
    }
};

/**
 * Main process application  
 */
void app_main(void){
    USER_GPIO_INIT();
    BaseType_t xReturned;

    xFila = xQueueCreate(5, sizeof(int));  //Cria a fila com 5 slots com tamanho de um int

    if( xFila == NULL){
    ESP_LOGE( "xFila", "Não foi possivel criar a fila"); 
    while(1);
    }

    xReturned = xTaskCreate(vTask_ADC,  "vTask_ADC",  configMINIMAL_STACK_SIZE + 1024,  NULL ,  1,  &xTask1Handle); 
    if( xReturned == pdFAIL){
        ESP_LOGE( "Task1", "Não foi possivel criar a Task 1"); 
        while(1);
    }

    xReturned = xTaskCreate(vTask_display,  "vTask_display",  configMINIMAL_STACK_SIZE + 1024,  NULL ,  1,  &xTask2Handle); 
    if( xReturned == pdFAIL){
        ESP_LOGE( "Task2", "Não foi possivel criar a Task 2"); 
        while(1);
    }
    
    xReturned = xTaskCreate(vTask_console,  "vTask_console",  configMINIMAL_STACK_SIZE + 1024,  NULL ,  1,  &xTask3Handle); 
    if( xReturned == pdFAIL){
        ESP_LOGE( "Task3", "Não foi possivel criar a Task 3"); 
        while(1);
    }
}