#include "comms/hwtimer.hpp"
#include "stdlib.hpp"

void zeroTcnt1() {
    //Disable interrupts
    noInterrupts();
    //Set TCNT1
    TCNT1 = 0;
    //Restore interrupts
    interrupts();
}

uint16_t readTcnt1() {
    //Disable interrupts
    noInterrupts();
    //Set TCNT1
    uint16_t i;
    i = TCNT1;
    //Restore interrupts
    interrupts();
    return i;
}
