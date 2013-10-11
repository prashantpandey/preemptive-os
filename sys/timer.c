/* File containing the functions to handle timer interrupt*/

#include <timer.h>
#include <idt.h>
#include <stdio.h>
#include <pic.h>
#include <common.h>

int tick = 0;
int total_time = 0;

// Will handle the timer interrupt when the interrupt is fired.
// Also will print the time elasped since the last reboot on the lower right corner of the screen
void timer_callback()
{
//	int len = 0;
	int hour = 0;
	int minute = 0;
	int second = 60;
   	tick++;
	outb(0x20, 0x20);	

	if(tick % 100 == 0) {
    		total_time++;
		int cursor_p_x = 30;
		int cursor_p_y = 24;
		
		second = total_time;			
		hour = (int)(second/3600);
		second = second - (hour*3600);
		minute = (int)(second/60);
		second = second - (minute*60);
	
		clear_line(cursor_p_y, cursor_p_x);	
		cursor_p_x = cursor_p_x + printf_string("TIME  ", cursor_p_y, cursor_p_x);
//		cursor_p_x = cursor_p_x + printf_string("Hour ", cursor_p_y, cursor_p_x);
		cursor_p_x = cursor_p_x + printf_int(hour, cursor_p_y, cursor_p_x);
		cursor_p_x = cursor_p_x + printf_string(":",cursor_p_y, cursor_p_x);
		cursor_p_x = cursor_p_x + printf_int(minute, cursor_p_y, cursor_p_x);
		cursor_p_x = cursor_p_x + printf_string(":", cursor_p_y, cursor_p_x);
		cursor_p_x = cursor_p_x + printf_int(second, cursor_p_y, cursor_p_x);

//		len = len + printf_string("Elapsed: ", cursor_p_y, cursor_p_x);
//		len = len + printf_int(total_time, cursor_p_y, (cursor_p_x + len));	
 	}
}

// Will initialize the timer interrupt 
void init_timer(uint32_t frequency)
{
	// The timer interrupt is registered in idt.c when the IDT is inilialized at position 32 mapped to 0 for PIC.

	// The value we send to the PIT is the value to divide it's input clock
   	// (1193180 Hz) by, to get our required frequency. Important to note is
   	// that the divisor must be small enough to fit into 16-bits.
   	uint32_t divisor = 1193180 / frequency;

   	// Send the command byte.
   	outb(0x43, 0x36);

   	// Divisor has to be sent byte-wise, so split here into upper/lower bytes.
   	uint8_t l = (uint8_t)(divisor & 0xFF);
   	uint8_t h = (uint8_t)( (divisor>>8) & 0xFF );

   	// Send the frequency divisor.
   	outb(0x40, l);
   	outb(0x40, h);
}
