# Sistema de Monitoramento com Raspberry Pi Pico W

Este projeto implementa um sistema de monitoramento de variáveis ambientais usando o Raspberry Pi Pico W. O sistema utiliza joystick analógico, display OLED, matriz de LEDs, buzzer e LEDs indicadores para representar visualmente os níveis de chuva e água. As tarefas são gerenciadas com FreeRTOS.

## Funcionalidades

- **Leitura de joystick (eixo X e Y):**
  - Conversão analógica via ADC.
  - Os valores simulam a leitura de nível de chuva (eixo X) e nível de água (eixo Y).

- **Display OLED (SSD1306):**
  - Mostra os percentuais dos níveis de chuva e água.
  - Exibe mensagens de alerta se os valores ultrapassarem limites pré-definidos.

- **LEDs indicadores:**
  - **Verde:** Condição normal.
  - **Vermelho:** Alerta de níveis elevados.

- **Matriz de LEDs WS2812 (5x5):**
  - Mostra um ícone de check (✓) para valores normais.
  - Mostra um ícone de exclamação (!) para situação de alerta.

- **Buzzer:**
  - Emite som quando os níveis estão fora do intervalo seguro.

- **Botão de reset:**
  - Ao pressionar o botão na GPIO 6, o sistema reinicia o Pico para o modo de boot.

## Componentes Utilizados

- Raspberry Pi Pico W
- Display OLED SSD1306 (I2C)
- Módulo de matriz de LEDs WS2812 (5x5)
- Joystick analógico
- 2 LEDs (verde e vermelho)
- Buzzer passivo
- Botão (reset)
- FreeRTOS para gerenciamento de tarefas

## Pinos Utilizados

| Componente      | GPIO       |
|------------------|------------|
| Joystick Eixo X  | GPIO 27 (ADC1) |
| Joystick Eixo Y  | GPIO 26 (ADC0) |
| LED Verde        | GPIO 11     |
| LED Vermelho     | GPIO 13     |
| Buzzer           | GPIO 10     |
| Botão de Reset   | GPIO 6      |
| Matriz WS2812    | GPIO 7 (com PIO) |
| Display OLED SDA | GPIO 14     |
| Display OLED SCL | GPIO 15     |

## Organização do Código

- `main.c`: Código principal com todas as tarefas FreeRTOS.
- `lib/ssd1306.h/.c`: Biblioteca para controle do display OLED.
- `lib/led_matriz.h/.c`: Biblioteca para desenho na matriz WS2812.
- `lib/buzzer.h/.c`: Função para ativação do buzzer.
- `pio_matriz.pio`: Código PIO para controle da matriz WS2812.
