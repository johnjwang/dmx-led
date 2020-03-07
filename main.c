#include "board.h"

int main()
{
    board_init();

    __enable_irq();

    while (1) {
    }
}

void rgb_init() {
    TCC0->WAVE.WAVEGEN = TCC_WAVE_WAVEGEN_NPWM;
    TCC0->PER.reg = 0xFF; //255
    TCC0->CTRLA.ENABLE = TCC_CTRLA_ENABLE;
}

void rgb_set() {

}
