#ifndef BOARD_H_
#define BOARD_H_

#include <stdint.h>

#include "samc20.h"


#define F_CPU       48001024ul

// Pin masks
#define PINS_AD     (0x1FFu << 2)
#define PIN_DMX_RX  (1u << 17)
#define PIN_GATE_R  (1u << 19)
#define PIN_GATE_G  (1u << 22)
#define PIN_GATE_B  (1u << 23)
#define PINS_GATE   (PIN_GATE_R | PIN_GATE_G | PIN_GATE_B)
#define PIN_LED     (1u << 24)


void board_init();
uint32_t board_utime32();

#endif // BOARD_H_
