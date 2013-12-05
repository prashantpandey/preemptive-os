/* File containing functions to handle Keyboard Interrupts */

#include <defs.h>
#include <print.h>
#include <pic.h>
#include <common.h>
#include <string.h>

char kbip[1024];			// Saves Keyboard input
int kbip_count = 0;			// No. of characters entered
volatile bool scanFlag = false;		// Tracks kscanf function call
bool kb_interrupt = false;		// Checks if keyboard interrupt has occured 
char input_char = ' ';			// Saves a single keyboard character
// global declaration of flag
static unsigned int flag = 0;

// Reset the keyboard input character array
void kbip_reset()
{
	int i = 0;
	for(i = 0;i<1024;i++)
	{
		kbip[i]='\0';
	}
}


/* Scan code mapping to US Layout keyboard */
unsigned char kbdus[128] =
{
    0,  27, '1', '2', '3', '4', '5', '6', '7', '8',	/* 9 */
  '9', '0', '-', '=', '\b',	/* Backspace */
  '\t',			/* Tab */
  'q', 'w', 'e', 'r',	/* 19 */
  't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',	/* Enter key */
    '^',			/* 29   - Control */
  'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';',	/* 39 */
 '\'', '`',   0,		/* Left shift */
 '\\', 'z', 'x', 'c', 'v', 'b', 'n',			/* 49 */
  'm', ',', '.', '/',   0,				/* Right shift */
  '*',
    0,	/* Alt */
  ' ',	/* Space bar */
    0,	/* Caps lock */
    0,	/* 59 - F1 key ... > */
    0,   0,   0,   0,   0,   0,   0,   0,
    0,	/* < ... F10 */
    0,	/* 69 - Num lock*/
    0,	/* Scroll Lock */
    0,	/* Home key */
    0,	/* Up Arrow */
    0,	/* Page Up */
  '-',
    0,	/* Left Arrow */
    0,
    0,	/* Right Arrow */
  '+',
    0,	/* 79 - End key*/
    0,	/* Down Arrow */
    0,	/* Page Down */
    0,	/* Insert Key */
    0,	/* Delete Key */
    0,   0,   0,
    0,	/* F11 Key */
    0,	/* F12 Key */
    0,	/* All other keys are undefined */
};		

unsigned char special_char[12] = {0,  27, '!', '@', '#', '$', '%', '^', '&', '*', '(', ')'};

/* Handles the keyboard interrupt */
void keyboard_handler()
{
	unsigned char scancode;
	int cursor_p_x = 8;
        int cursor_p_y = 24;
	int len = 0;
	kb_interrupt = true;
	//char input_char = ' ';
	/* Read from the keyboard's data buffer */
    	scancode = inb(0x60);

    	/* If the top bit of the byte we read from the keyboard is
    	*  set, that means that a key has just been released */
    	if (scancode & 0x80)
    	{
        	/* You can use this one to see if the user released the
        	*  shift, alt, or control keys... */
    	}
   	else
    	{
        	/* Here, a key was just pressed. Please note that if you
        	*  hold a key down, you will get repeated key press
        	*  interrupts. */

        	/* Just to show you how this works, we simply translate
        	*  the keyboard scancode into an ASCII value, and then
        	*  display it to the screen. You can get creative and
        	*  use some flags to see if a shift is pressed and use a
        	*  different layout, or you can add another 128 entries
        	*  to the above layout to correspond to 'shift' being
        	*  held. If shift is held using the larger lookup table,
        	*  you would add 128 to the scancode when you look for it */
		
		len = printf_string("Current Key Press:", cursor_p_y, cursor_p_x + len);
		
		if(flag == 0 && ((int)(scancode)) == 28)
		{
					
			handle_enter();
			input_char = kbdus[scancode];
		
			// Set ScanFlag as false since enter has been encountered
			scanFlag = false;	
			
		}
		else if(flag == 0 && ((int)(scancode)) == 14)
		{
			handle_backspace();
		}		
		else if(flag == 0 && ((int)(scancode)) != 42) 	// Will check for case when shift key is not pressed
		{
			input_char = kbdus[scancode];
			kprintf("%c", kbdus[scancode]);
			printf_char(kbdus[scancode], cursor_p_y, cursor_p_x + len);
		}		
		else if(flag == 1 && ((int) scancode) < 12)	//Will check if shift key is pressed along with numbers
		{
			input_char =  special_char[scancode];
			kprintf("%c", special_char[scancode]);
			printf_char(special_char[scancode], cursor_p_y, cursor_p_x + len);
			flag = 0;
		}
		else if(flag == 1) 				//Will check if shift key is pressed along with alphabets
		{
			int code = (((int) (kbdus[scancode])) - 32);	
			input_char = code;
			kprintf("%c", code);
			printf_char(code, cursor_p_y, cursor_p_x + len);
			flag = 0;
		}
		else if(((int)(scancode)) == 42) {		//Will check for shift key press and set the flag to 1
			flag = 1;	
		}
	
		// Buffers input characters if kscanf is called and entered character is not new line
		if(scanFlag == true && input_char!='\n')
			{
			kbip[kbip_count++] = input_char;
			}
    	}
	outb(0x20, 0x20);
}


int kscanf(char* buffer) {

	int i = 0;
	scanFlag = true;
	kbip_count = 0;

	// va_start(args, fmt);
	
	kbip_reset();	
	while(scanFlag);
	for(i = 0; i < 512; i++) {
		buffer[i] = '\0';
	}
	kbip[kbip_count] = '\0';
//	kprintf("Reaching here");
	strcpy(buffer, kbip);
	/*while(*buffer!='\0')
	{
	kprintf("%c",*buffer++);
	}*/
	kbip_reset();
	
/*
	while(1)
	{
	
		if(scanFlag == false)
		{
			kprintf("\n %d\n", kbip_count);
		//	kprintf("Reaching here");
		//	kprintf("\n %c", *kbip);
			switch(*fmt)
			{	
			case 'c':
					*var++ = kbip[0];
					*var++ = '\0';
					break;
			case 's':
					for (i = 0; i< kbip_count;i++)
					{
						*var++ = kbip[i];
						kprintf("%c", kbip[i]);
					}
					*var++ = '\0';
					break;
			case 'd':
	
					for (i = 0; i< kbip_count;i++)
					{
						*var++ = kbip[i];
						kprintf("%c", kbip[i]);
					}
					*var++ = '\0';
					int result = stoi(var);
					kprintf("%d",result);
					break;
			default:	
					break;
			}
			kbip_reset();
		break;
		}
	}
*/
	return 0;
}
