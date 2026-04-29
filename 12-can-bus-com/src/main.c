#include "stm32f407.h"
#include "gpio.h"
#include "uart.h"
#include "can.h"

// Simple delay using the DWT cycle counter.
// Assumes default 16 MHz HSI clock at boot (no PLL configuration).
#define SYSCLK_HZ  16000000UL
void SVC_Handler(void)     { while (1) { } }
void PendSV_Handler(void)  { while (1) { } }
void SysTick_Handler(void) { while (1) { } }

static void delay_init(void) {
    CoreDebug->DEMCR |= (1 << 24);   // TRCENA: enable trace
    DWT->CTRL |= (1 << 0);           // CYCCNTENA: enable cycle counter
    DWT->CYCCNT = 0;
}

static void delay_ms(uint32_t ms) {
    uint32_t start = DWT->CYCCNT;
    uint32_t cycles = ms * (SYSCLK_HZ / 1000);
    while ((DWT->CYCCNT - start) < cycles) { }
}

// Print a received frame as text over UART (no printf needed).
static void print_rx_frame(const can_frame_t *f) {
    uart_write_string("RX id=0x");
    uart_write_hex((uint8_t)((f->id >> 8) & 0xFF));
    uart_write_hex((uint8_t)(f->id & 0xFF));
    uart_write_string(" dlc=");
    uart_write_hex(f->dlc);
    uart_write_string(" data=");
    for (uint8_t i = 0; i < f->dlc && i < 8; i++) {
        uart_write_hex(f->data[i]);
        uart_write_char(' ');
    }
    uart_write_string("\r\n");
}

int main(void) {
    delay_init();
    uart_init(115200);

    // LED on PD12 (Discovery green LED)
    gpio_init(GPIOD, 12, GPIO_MODE_OUTPUT);

    uart_write_string("Booting CAN loopback test\r\n");

    // Init CAN in loopback silent mode at 500 kbps
    if (can_init(CAN_BITRATE_500K, CAN_MODE_LOOPBACK_SILENT) != 0) {
        uart_write_string("CAN init FAILED\r\n");
        // Blink LED rapidly to indicate failure
        while (1) {
            toggle_pin(GPIOD, 12);
            delay_ms(100);
        }
    }

    can_filter_accept_all();
    uart_write_string("CAN ready\r\n");

    can_frame_t tx_frame = {
        .id = 0x123,
        .ide = 0,
        .rtr = 0,
        .dlc = 4,
        .data = {0xDE, 0xAD, 0xBE, 0xEF, 0, 0, 0, 0}
    };

    can_frame_t rx_frame;

    while (1) {
        can_transmit(&tx_frame);
        delay_ms(10);

        if (can_receive_available()) {
            if (can_receive(&rx_frame) == 0) {
                toggle_pin(GPIOD, 12);
                print_rx_frame(&rx_frame);
            }
        }

        delay_ms(500);
    }
}