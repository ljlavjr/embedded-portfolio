#include "stm32f407.h"
#include "iwdg.h"

void iwdg_init(uint16_t reload, uint8_t prescaler) {
    // Unlock PR and RLR registers for write access
    // Must be written before any configuration changes
    IWDG->KR = IWDG_KEY_UNLOCK;

    // Set prescaler divider (0-6 maps to /4, /8. /16, ... /256)
    // Timeout = (prescaler_divider * reload) / 32000Hz (LSI clock)
    IWDG->PR = prescaler;

    // Load countdown value (12-bit, 0-4095)
    // Counter resets to this value on each refresh (kick)
    IWDG->RLR = reload;
    
    // Wait for hardware to finish applying prescaler (PVU, bit 0)
    // and reload (RVU, bit 1) updates before proceeding
    // Bits are SET while updating, CLEAR when ready
    while(((0b11 << 0) & IWDG->SR));
    return;
}

void iwdg_start(void) {
    // Start the watchdog.
    // There is no stopping it now.
    IWDG->KR = IWDG_START;
}

void iwdg_refresh(void) {
    // Refreshes (kick) the watchdog to let it know that
    // tasks are running as intended
    IWDG->KR = IWDG_KICK;
}

bool iwdg_check(void) {
    // Read Bit 29 (IWDGRSTF) to check for iwdg reset
    // Save the state
    uint32_t resetFlag = (RCC->CSR & (1 << 29));

    // Remove the reset flag
    RCC->CSR |= (1 << 24);

    // Return if watchdog was reset
    if (resetFlag) {
        return true;
    }
    else {
        return false;
    }
}