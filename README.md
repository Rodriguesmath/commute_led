# ESP32: Controle de LED com Auto-Off e Debounce (FSM)

Este projeto demonstra a implementação de uma lógica de controle de entrada e saída digital utilizando o framework **ESP-IDF**. O sistema alterna o estado de um LED através de um botão físico, utilizando uma **Máquina de Estados Finitos (FSM)** para o *debounce* e um temporizador para desligamento automático (*auto-off*).

---

## 🛠️ Especificações de Hardware

Com base no esquema elétrico, o projeto utiliza a seguinte configuração:

### Componentes e Conexões
*   **MCU:** ESP32
*   **Saída (LED):** Conectado ao **GPIO 15** com um resistor limitador de $330\Omega$.
*   **Entrada (Botão):** Conectado ao **GPIO 4**. 
    *   **Configuração:** Pull-up externo de $10k\Omega$.
    *   **Lógica:** Ativo em nível baixo (GND quando pressionado).

### Esquema de Ligação
*   **R1 (330Ω):** Entre GPIO 15 e Anodo do LED.
*   **R2 (10kΩ):** Entre 3V3 e GPIO 4 (Resistor de Pull-up).
*   **SW-1:** Entre GPIO 4 e GND.

---

## 🚀 Funcionalidades do Software

O código foi estruturado seguindo boas práticas de sistemas embarcados, evitando o uso de `vTaskDelay` bloqueante para a lógica de debounce.

### 1. Máquina de Estados do Botão (Debounce)
Para evitar o fenômeno de "bouncing" (ruído mecânico), implementamos quatro estados:
*   **IDLE:** Aguardando sinal baixo no pino.
*   **DEBOUNCE:** Aguarda 50ms para confirmar se a pressão é real.
*   **PRESSED:** Confirma o clique e dispara o evento.
*   **WAIT_RELEASE:** Aguarda o usuário soltar o botão para permitir um novo ciclo.

### 2. Controle de Temporização (Auto-Off)
Ao ligar o LED, o sistema registra o tempo inicial (`xTaskGetTickCount`). Se o LED permanecer ligado por mais de **10 segundos** sem uma nova interação, a função de controle força o estado para `false`.

---

## 💻 Estrutura do Código

O arquivo principal contém:
*   `bsp_gpio_init()`: Configuração dos drivers GPIO.
*   `button_poll_event()`: Processador da FSM de entrada.
*   `system_control_task_handler()`: Lógica de negócio (Toggle e Timer).
*   `app_main()`: Loop principal com polling a cada 10ms.

---

## ⚙️ Como Compilar e Gravar

1.  Certifique-se de ter o **ESP-IDF** (v4.4 ou superior) configurado.
2.  Clone este repositório.
3.  No terminal, execute:

```bash
# Configurar o alvo
idf.py set-target esp32

# Compilar, gravar e abrir o monitor
idf.py build flash monitor