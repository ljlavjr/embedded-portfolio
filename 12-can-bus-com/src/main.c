#include "stm32f407.h"
#include "gpio.h"
#include "uart.h"
#include "can.h"
#include "timing.h"

int main(void) {
    timing_init();
    // UART for debug output
    uart_init(115200);

    // LED on PD12 (or whichever LED on your discovery board)
    gpio_init(GPIOD, 12, GPIO_MODE_OUTPUT);

    // Init CAN in loopback silent mode at 500 kbps
    if (can_init(CAN_BITRATE_500K, CAN_MODE_LOOPBACK_SILENT) != 0) {
        // CAN init failed, blink LED rapidly
        while (1) {
            toggle_pin(GPIOD, 12);
            delay_ms(100);
        }
    }

    can_filter_accept_all();

    can_frame_t tx_frame = {
        .id = 0x123,
        .ide = 0,
        .rtr = 0,
        .dlc = 4,
        .data = {0xDE, 0xAD, 0xBE, 0xEF, 0, 0, 0, 0}
    };

    can_frame_t rx_frame;

    while (1) {
        // Transmit a frame
        can_transmit(&tx_frame);

        // Wait briefly for it to loop back
        delay_ms(10);

        // Check for a received frame
        if (can_receive_available()) {
            if (can_receive(&rx_frame) == 0) {
                toggle_pin(GPIOD, 12);
                uart_printf(USART2, "RX id=0x%X dlc=%d data=%02X %02X %02X %02X\r\n",
                            rx_frame.id, rx_frame.dlc,
                            rx_frame.data[0], rx_frame.data[1],
                            rx_frame.data[2], rx_frame.data[3]);
            }
        }

        delay_ms(500);
    }
}