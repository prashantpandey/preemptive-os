#ifndef _PIC_H
#define _PIC_H

//#include <unistd.h>
#include <defs.h>

void pic_remap(uint8_t offset1, uint8_t offset2);
void outb( unsigned short port, unsigned char val );
unsigned char inb( unsigned short port );
void pic_send_eoi(unsigned char irq);


#endif

