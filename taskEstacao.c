#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "hardware/gpio.h"
#include "hardware/i2c.h"
#include "hardware/pwm.h"
#include "pico/bootrom.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "lib/ssd1306.h"
#include "lib/buzzer.h"
#include "lib/led_matriz.h"
#include "lib/font.h"
#include "pio_matriz.pio.h"
#include <stdio.h>

#define SDA_PIN 14
#define SCL_PIN 15
#define I2C_BUS i2c1
#define I2C_ADDR 0x3C
#define JOY_X_PIN 26
#define JOY_Y_PIN 27
#define GREEN_LED 11
#define RED_LED 13
#define BUZZER_PIN 10
#define BTN_B 6

#define LED_SIZE 10

ssd1306_t display;
bool fundo_preto = true;

typedef struct {
    uint16_t eixo_x;
    uint16_t eixo_y;
} dados_joystick_t;

QueueHandle_t filaJoystick;

void taskLeituraJoystick(void *parametros) {
    adc_gpio_init(JOY_Y_PIN);
    adc_gpio_init(JOY_X_PIN);
    adc_init();

    dados_joystick_t leitura;

    while (1) {
        adc_select_input(0);
        leitura.eixo_y = adc_read();

        adc_select_input(1);
        leitura.eixo_x = adc_read();

        xQueueSend(filaJoystick, &leitura, 0);
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

void taskDisplay(void *parametros) {
    dados_joystick_t leitura;

    while (1) {
        if (xQueueReceive(filaJoystick, &leitura, portMAX_DELAY) == pdTRUE) {
            uint16_t percX = leitura.eixo_x * 100 / 4095;
            uint16_t percY = leitura.eixo_y * 100 / 4095;

            char texto_chuva[5];
            char texto_nivel[5];

            sprintf(texto_chuva, "%d", percX);
            sprintf(texto_nivel, "%d", percY);

            ssd1306_fill(&display, !fundo_preto);
            if (leitura.eixo_x >= 3480 || leitura.eixo_y >= 3071) {
                ssd1306_draw_string(&display, "ALERTA!", centralizar_texto("ALERTA!"), 5);
                ssd1306_draw_string(&display, "NIVEIS ANORMAIS", centralizar_texto("NIVEIS ANORMAIS"), 15);
            } else {
                ssd1306_draw_string(&display, "Niveis normais", centralizar_texto("Niveis Normais"), 15);
            }

            ssd1306_draw_string(&display, "V. chuva:", 10, 35);
            ssd1306_draw_string(&display, texto_chuva, 90, 35);
            ssd1306_draw_string(&display, "%", 110, 35);

            ssd1306_draw_string(&display, "N. agua:", 10, 45);
            ssd1306_draw_string(&display, texto_nivel, 90, 45);
            ssd1306_draw_string(&display, "%", 110, 45);

            ssd1306_send_data(&display);
        }
    }
}

void taskLeds(void *parametros) {
    dados_joystick_t leitura;
    while (1) {
        if (xQueueReceive(filaJoystick, &leitura, portMAX_DELAY) == pdTRUE) {
            printf("X: %d, Y: %d\n", leitura.eixo_x, leitura.eixo_y);
            if (leitura.eixo_x >= 3480 || leitura.eixo_y >= 3070) {
                gpio_put(GREEN_LED, false);
                gpio_put(RED_LED, true);
            } else {
                gpio_put(GREEN_LED, true);
                gpio_put(RED_LED, false);
            }
        }
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}

void taskMatriz(void *parametros) {
    dados_joystick_t leitura;
    PIO pio = pio0;
    uint sm = 0;
    uint offset = pio_add_program(pio, &pio_matriz_program);
    pio_matriz_program_init(pio, sm, offset, pino_matriz);

    while (1) {
        if (xQueueReceive(filaJoystick, &leitura, portMAX_DELAY) == pdTRUE) {
            if (leitura.eixo_x >= 3480 || leitura.eixo_y >= 3071) {
                exclamacao();
            } else {
                checkmark();
            }
            desenho_pio(0, pio, sm);
        }
    }
}

void taskBuzzer(void *parametros) {
    dados_joystick_t leitura;

    while (1) {
        if (xQueueReceive(filaJoystick, &leitura, portMAX_DELAY) == pdTRUE) {
            if (leitura.eixo_x >= 3480 || leitura.eixo_y >= 3070) {
                buzz(BUZZER_PIN, 600, 500);
                for (int i = 0; i < 10; i++)
                    vTaskDelay(pdMS_TO_TICKS(10));
            }
        }
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}

void handlerBotaoBoot(uint gpio, uint32_t eventos) {
    PIO pio = pio0;
    uint sm = 0;
    uint offset = pio_add_program(pio, &pio_matriz_program);
    pio_matriz_program_init(pio, sm, offset, pino_matriz);

    ssd1306_fill(&display, !fundo_preto);
    ssd1306_send_data(&display);

    limpar_todos_leds();
    desenho_pio(0, pio, sm);

    reset_usb_boot(0, 0);
}

void inicializarSistema() {
    gpio_init(GREEN_LED);
    gpio_set_dir(GREEN_LED, GPIO_OUT);
    gpio_put(GREEN_LED, false);

    gpio_init(RED_LED);
    gpio_set_dir(RED_LED, GPIO_OUT);
    gpio_put(RED_LED, false);

    gpio_init(BUZZER_PIN);
    gpio_set_dir(BUZZER_PIN, GPIO_OUT);

    gpio_init(BTN_B);
    gpio_set_dir(BTN_B, GPIO_IN);
    gpio_pull_up(BTN_B);
    gpio_set_irq_enabled_with_callback(BTN_B, GPIO_IRQ_EDGE_FALL, true, &handlerBotaoBoot);

    i2c_init(I2C_BUS, 400 * 1000);
    gpio_set_function(SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(SDA_PIN);
    gpio_pull_up(SCL_PIN);

    ssd1306_init(&display, WIDTH, HEIGHT, false, I2C_ADDR, I2C_BUS);
    ssd1306_config(&display);
    ssd1306_send_data(&display);
}

int main() {
    inicializarSistema();
    stdio_init_all();

    filaJoystick = xQueueCreate(5, sizeof(dados_joystick_t));

    xTaskCreate(taskLeituraJoystick, "Joystick", 256, NULL, 1, NULL);
    xTaskCreate(taskDisplay, "Display", 512, NULL, 1, NULL);
    xTaskCreate(taskLeds, "Leds", 256, NULL, 1, NULL);
    xTaskCreate(taskMatriz, "Matriz", 256, NULL, 1, NULL);
    xTaskCreate(taskBuzzer, "Buzzer", 256, NULL, 1, NULL);

    vTaskStartScheduler();
    panic_unsupported();
}