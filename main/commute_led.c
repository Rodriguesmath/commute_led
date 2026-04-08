#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"

/* --- Hardware Abstraction Layer (HAL) Definitions --- */
#define GPIO_OUTPUT_LED_BITMASK ((1ULL << 15))
#define GPIO_INPUT_BTN_BITMASK  ((1ULL << 4))

#define PIN_LED 15
#define PIN_BTN   4 

/* --- Timing & Logic Constants --- */
#define BTN_DEBOUNCE_MS        50
#define AUTO_OFF_DELAY_MS      10000

typedef enum {
    BUTTON_STATE_IDLE,
    BUTTON_STATE_DEBOUNCE,
    BUTTON_STATE_PRESSED,
    BUTTON_STATE_WAIT_RELEASE
} button_state_t;

typedef struct {
    gpio_num_t gpio_num;
    button_state_t current_state;
    TickType_t debounce_start_tick;
} button_handle_t;

/* --- Private Global Variables --- */
static button_handle_t g_btn_light_handle = { .gpio_num = PIN_BTN, .current_state = BUTTON_STATE_IDLE };

static bool g_led_state = false;       
static TickType_t g_led_on_tick = 0;   

/* --- Module Functions --- */

/**
 * @brief Inicializa os periféricos GPIO seguindo as especificações de driver externo.
 */
void bsp_gpio_init(void) {
    gpio_config_t led_cfg = {
        .pin_bit_mask = GPIO_OUTPUT_LED_BITMASK,
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE // Optei por usar pull-up externo na minha montagem
    };
    gpio_config(&led_cfg);

    gpio_config_t btn_cfg = {
        .pin_bit_mask = GPIO_INPUT_BTN_BITMASK,
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE // Optei por usar pull-up externo na minha montagem
    };
    gpio_config(&btn_cfg);
}



/**
 * @brief Gerencia a máquina de estados finitos para debounce de entrada digital.
 * @return true se um evento de clique válido for detectado (borda de descida).
 */
bool button_poll_event(button_handle_t *handle) {
    int raw_level = gpio_get_level(handle->gpio_num);
    TickType_t current_tick = xTaskGetTickCount();
    bool is_triggered = false;

    switch (handle->current_state) {
        case BUTTON_STATE_IDLE:
            if (raw_level == 0) { 
                handle->debounce_start_tick = current_tick;
                handle->current_state = BUTTON_STATE_DEBOUNCE;
            }
            break;

        case BUTTON_STATE_DEBOUNCE:
            if ((current_tick - handle->debounce_start_tick) >= pdMS_TO_TICKS(BTN_DEBOUNCE_MS)) {
                if (gpio_get_level(handle->gpio_num) == 0) {
                    handle->current_state = BUTTON_STATE_PRESSED;
                    is_triggered = true;
                } else {
                    handle->current_state = BUTTON_STATE_IDLE;
                }
            }
            break;

        case BUTTON_STATE_PRESSED:
            if (raw_level == 1) {
                handle->current_state = BUTTON_STATE_WAIT_RELEASE;
            }
            break;

        case BUTTON_STATE_WAIT_RELEASE:
            handle->current_state = BUTTON_STATE_IDLE;
            break;
    }
    return is_triggered;
}

/**
 * @brief Executa a lógica de comutar o estado do led e apagar automaticamente.
 */
void system_control_task_handler(void) {
    TickType_t current_tick = xTaskGetTickCount();

    if (button_poll_event(&g_btn_light_handle)) {
        g_led_state = !g_led_state; 
        
        if (g_led_state) {
            g_led_on_tick = current_tick;
        } else {
        }
        
        gpio_set_level(PIN_LED, g_led_state);
    }

    if (g_led_state) {
        if ((current_tick - g_led_on_tick) >= pdMS_TO_TICKS(AUTO_OFF_DELAY_MS)) {
            g_led_state = false;
            gpio_set_level(PIN_LED, g_led_state);
        }
    }
}

void app_main(void) {
    bsp_gpio_init();
    gpio_set_level(PIN_LED, 0);

    while (1) {
        system_control_task_handler();
        
        vTaskDelay(pdMS_TO_TICKS(10)); 
    }
}