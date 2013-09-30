#ifndef _IDT_H
#define _IDT_H

//#include <unistd.h>
#include <defs.h>

void init_idt();
void idt_set_gate(int num, uint64_t base, uint16_t sel, uint8_t flags);
void outportb(unsigned short _port, unsigned char _data);

#endif



