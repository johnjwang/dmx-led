#include "board.h"


static void port_configure(PortGroup *port, uint32_t pinmask, uint32_t wrconfig)
{
    // Mask out the HWSEL, WRite, and PINMASK bits
    wrconfig &= PORT_WRCONFIG_PMUXEN | PORT_WRCONFIG_INEN |
        PORT_WRCONFIG_PULLEN | PORT_WRCONFIG_DRVSTR | PORT_WRCONFIG_PMUX_Msk;
    // Always write both peripheral mux and pin configuration
    wrconfig |= PORT_WRCONFIG_WRPMUX | PORT_WRCONFIG_WRPINCFG;

    if (pinmask & 0xFFFF) {
        port->WRCONFIG.reg = wrconfig | PORT_WRCONFIG_PINMASK(pinmask);
    }
    if (pinmask >> 16) {
        port->WRCONFIG.reg = wrconfig | PORT_WRCONFIG_HWSEL |
            PORT_WRCONFIG_PINMASK(pinmask >> 16);
    }
}

void board_init()
{
    ///////////////////////////////////////////////////////
    // CLOCK
    ///////////////////////////////////////////////////////

    // Enable 32.768kHz external crystal
    OSC32KCTRL->XOSC32K.reg = OSC32KCTRL_XOSC32K_ENABLE |
        OSC32KCTRL_XOSC32K_XTALEN | OSC32KCTRL_XOSC32K_EN32K;
    while (!(OSC32KCTRL->STATUS.reg & OSC32KCTRL_STATUS_XOSC32KRDY));

    // Connect XOSC32K to GCLK 2
    GCLK->GENCTRL[2].reg = GCLK_GENCTRL_DIV(1) | GCLK_GENCTRL_SRC_XOSC32K |
        GCLK_GENCTRL_GENEN;

    // Enable PLL at 48 MHz (32.768 kHz * (1464 + 14/16) = 48001024 Hz)
    OSCCTRL->DPLLRATIO.reg = OSCCTRL_DPLLRATIO_LDR(1463) |
        OSCCTRL_DPLLRATIO_LDRFRAC(14);
    OSCCTRL->DPLLCTRLA.reg = OSCCTRL_DPLLCTRLA_ENABLE;
    while (OSCCTRL->DPLLSYNCBUSY.reg & OSCCTRL_DPLLSYNCBUSY_ENABLE);
    while (!(OSCCTRL->DPLLSTATUS.reg & OSCCTRL_DPLLSTATUS_LOCK));

    // Set flash waitstates to 2 before switching clock (per atmel sample code)
    NVMCTRL->CTRLB.bit.RWS = 2;

    // Set clock 0 (and CPU core) to 48 MHz
    GCLK->GENCTRL[0].reg = GCLK_GENCTRL_DIV(1) | GCLK_GENCTRL_SRC_DPLL96M |
        GCLK_GENCTRL_GENEN;
    // Set clock 3 to 1 MHz
    GCLK->GENCTRL[3].reg = GCLK_GENCTRL_DIV(48) | GCLK_GENCTRL_SRC_DPLL96M |
        GCLK_GENCTRL_GENEN;

    ///////////////////////////////////////////////////////
    // TIMER
    ///////////////////////////////////////////////////////

    // Enable TC0 and TC1 clock at 1MHz
    MCLK->APBCMASK.reg |= MCLK_APBCMASK_TC0 | MCLK_APBCMASK_TC1;
    GCLK->PCHCTRL[TC0_GCLK_ID].reg = GCLK_PCHCTRL_CHEN | GCLK_PCHCTRL_GEN(3);

    // Reset timers
    TC0->COUNT32.CTRLA.reg = TC_CTRLA_SWRST;
    TC1->COUNT32.CTRLA.reg = TC_CTRLA_SWRST;
    while (TC0->COUNT32.CTRLA.reg & TC_CTRLA_SWRST);
    while (TC1->COUNT32.CTRLA.reg & TC_CTRLA_SWRST);

    // Enable TC0 in 32-bit mode (this uses TC1 as the upper 16 bits)
    TC0->COUNT32.CTRLA.reg = TC_CTRLA_MODE_COUNT32 | TC_CTRLA_ENABLE;
    while (TC0->COUNT32.SYNCBUSY.reg & TC_SYNCBUSY_ENABLE);

    ///////////////////////////////////////////////////////
    // PIN CONFIG
    ///////////////////////////////////////////////////////

    // Configure address pins as inputs with pull-up
    PORT->Group[0].DIRCLR.reg = PINS_AD;
    PORT->Group[0].OUTSET.reg = PINS_AD;
    port_configure(&PORT->Group[0], PINS_AD,
                   PORT_WRCONFIG_PULLEN | PORT_WRCONFIG_INEN);

    // Assign DMX RX pin to peripheral C (SERCOM1)
    PORT->Group[0].DIRCLR.reg = PIN_DMX_RX;
    PORT->Group[0].OUTSET.reg = PIN_DMX_RX;
    port_configure(&PORT->Group[0], PIN_DMX_RX,
                   PORT_WRCONFIG_PULLEN | PORT_WRCONFIG_INEN |
                   PORT_WRCONFIG_PMUXEN | PORT_WRCONFIG_PMUX(2));

    // Configure status LED as output
    PORT->Group[0].DIRSET.reg = PIN_LED;
    PORT->Group[0].OUTCLR.reg = PIN_LED;

    // Assign gate drive pins to peripheral F (TCC0) with high drive strength
    PORT->Group[0].DIRSET.reg = PINS_GATE;
    PORT->Group[0].OUTCLR.reg = PINS_GATE;
    port_configure(&PORT->Group[0], PINS_GATE, PORT_WRCONFIG_DRVSTR |
                   PORT_WRCONFIG_PMUXEN | PORT_WRCONFIG_PMUX(5));
}

uint32_t board_utime32()
{
    TC0->COUNT32.CTRLBSET.reg = TC_CTRLBSET_CMD_READSYNC;
    while (TC0->COUNT32.CTRLBSET.bit.CMD);
    while (TC0->COUNT32.SYNCBUSY.reg & TC_SYNCBUSY_COUNT);

    return TC0->COUNT32.COUNT.reg;
}
