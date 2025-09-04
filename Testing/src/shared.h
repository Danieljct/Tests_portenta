#ifndef SHARED_H
#define SHARED_H

#define BUF_SIZE 128

// Buffer circular simple en SRAM compartida
struct SharedBuffer {
    volatile char data[BUF_SIZE];
    volatile int head;
    volatile int tail;
};


// Indica al compilador que la variable está en SRAM D1 (compartida)
#if defined(CORE_CM4) || defined(CORE_CM7)
__attribute__((section(".ccmram"))) extern volatile SharedBuffer msgBuffer;
#endif

#endif

