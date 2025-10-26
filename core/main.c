#include "libc/base.h"

void irq_install() {
    asm volatile("sti");
    init_main(50);
}

int _start() {
    isr_install();
    irq_install();
    return 0;
}