#ifndef _PIC_H
#define _PIC_H

//#include <unistd.h>
#include <defs.h>

void pic_remap(uint8_t offset1, uint8_t offset2);
void pic_send_eoi(unsigned char irq);


#endif

