/*
Criar um projeto com FreeRTOS com 3 Tasks. As tasks devem executar as seguintes ações:
    Task 1: Piscar um LED com intervalo definido na criação da Task
    Task 2: Imprimir o nome dado para a task e em seguida ser deletada
    Task 3: Imprimir uma mensagem passada através de uma struct. A mensagem deve conter um ID e uma string.
*/
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "string.h"

#define ESP32_BLINK_GPIO 13
#define MSG_BUFFER_SIZE	50

static uint8_t s_led_state = 0;

/* GLOBAL defines */
typedef struct {
    int ID;
    char MSN[MSG_BUFFER_SIZE];
}vTask3_data;


TaskHandle_t xTask1Handle, xTask2Handle, xTask3Handle;
void vTask1(void *pvParameters); 
void vTask2(void *pvParameters); 
void vTask3(vTask3_data *DATA);

/**
 * GPIOs configuration and initialize 
 */
void USER_GPIO_INIT(void)
{
    //insert atributes for tactile button 
    gpio_config_t led_user_config = {
        .intr_type = GPIO_INTR_DISABLE,
        .mode = GPIO_MODE_OUTPUT,
        .pin_bit_mask = (1ULL << ESP32_BLINK_GPIO),
        .pull_down_en = 0
    };
    //initilize tactile button 
    gpio_config(&led_user_config);
};

/**
 * Task 1: Piscar um LED com intervalo definido na criação da Task
 */
void vTask1(void *pvParameters)
{
    int set_delay = (int)pvParameters;
    while(1)
    {
        s_led_state = !s_led_state;
        /* Set the GPIO level according to the state (LOW or HIGH)*/
        gpio_set_level(ESP32_BLINK_GPIO, s_led_state);
        vTaskDelay(set_delay / portTICK_PERIOD_MS);
    }
}

/**
 * Task 2: Imprimir o nome dado para a task e em seguida ser deletada
 */
void vTask2(void *pvParameters)
{
    printf("vTaskName Process = %s \n", pcTaskGetTaskName(xTask2Handle));
    vTaskDelete(xTask2Handle); 
}

/**
 * Task 3: Imprimir uma mensagem passada através de uma struct. A mensagem deve conter um ID e uma string.
 */
void vTask3(vTask3_data *DATA)
{
    printf("*** ID (received)  = %d \n", DATA->ID);
    printf("*** MSN (received) = %s \n", DATA->MSN);
}

/**
 * Main process application  
 */
void app_main(void){
    USER_GPIO_INIT();
    
    vTask3_data vTask3_send_data;
    vTask3_send_data.ID = 123;
    snprintf(vTask3_send_data.MSN, MSG_BUFFER_SIZE, "Mensagem da Task3");

    xTaskCreate(vTask1, "Task1", configMINIMAL_STACK_SIZE + 1024, (void*)100, 1,  &xTask1Handle);    
    xTaskCreate(vTask2, "Task2", configMINIMAL_STACK_SIZE + 1024, NULL, 2,  &xTask2Handle);    
    xTaskCreate(vTask3, "Task3", configMINIMAL_STACK_SIZE + 1024, &vTask3_send_data, 3, &xTask3Handle);    
}
