/*
The file containing the functions for printf
*/
#include <print.h>
#include <stdarg.h>
#include <paging.h>
#include <common.h>
#include <string.h>

#define WHITE_TXT 0x07 // white on black text
#define VIDMEM (0xb8000 + KERNBASE)  // start address of the video memory


// global declaration of cursor position
int cursor_p_x = 0;
int cursor_p_y = 0;

/** 
 * generic utility functions 
 */

/*
// Will return the length of the string
unsigned int strlen(const char *str)
{
        int i = 0;
        while(str[i] != '\0')
                i++;
        return i;
}

char* strcpy(char *dst, const char *src) {
	char *ret;
	ret = dst;
	while ((*dst++ = *src++) != '\0')
	return ret;
}
*/

// Will set the int array to 0's
unsigned int int_array_reset(int array[], int cnt)
{
        int i = 0;
        for(i = 0; i < cnt; i++) {
                array[i] = 0;
        }
        return (1);
}

// Will set the char array to '\0'
unsigned int char_array_reset(char array[], int cnt)
{
        int i = 0;
        for(i = 0; i < cnt; i++) {
                array[i] = '\0';
        }
        return (1);
}

// Will reverse the char array 
void char_array_reverse(char array[], int cnt, char *final)
{
        int i = 0;
        for(i = 0; i <= cnt; i++)
        {
                final[i] = array[cnt];
                cnt--;
        }

        final[++i] = '\0';
}

// Will clear the rest of the line from the given cursor position
void clear_line(unsigned int line, unsigned int column) 
{
	char *vidmem = (char *) VIDMEM;
        unsigned int i = 0;
	int boundary = 0;
        i = ((line*80) + column)*2;     // y-axis * width_of_screen + x-axis (offset)   
	
	boundary = i + (80-column);
        while(i <= boundary)
        {
                vidmem[i] = ' ';
                i++;
                vidmem[i] = WHITE_TXT;
                i++;
        }
}

// Will start the scroller
void start_scroller()
{
	int i = 0;
	int j = 0;
        char *vidmem = (char *) VIDMEM;
	int pos1 = 0;
	int pos2 = 0;

       	for(i = 2; i < 24; i++)
       	{
       		for(j = 0; j < 80; j++)
                {
	              	pos1 = (((i-1)*80) + j)*2;
                        pos2 = ((i*80) + j)*2;
                        vidmem[pos1++] = vidmem[pos2++];
			vidmem[pos1] = vidmem[pos2];
                }
		clear_line(i, 0);
        }
	cursor_p_x = 0;
}


/**
 * Generic functions to perform modular tasks. 
 */

// Will spit out the character at the given line number
unsigned int printf_char(char message, unsigned int line, unsigned int column) {
        char *vidmem = (char *) VIDMEM;
        unsigned int i = 0;
        int initial_p = 0;

        i = ((line*80) + column)*2;     // y-axis * width_of_screen + x-axis (offset)   
        initial_p = i;

        vidmem[i] = message;
        i++;
        vidmem[i] = WHITE_TXT;
        i++;

        return((int)(i - initial_p)/2); // will return the number of memory locations used
}

// Will spit out the string message at the given line number
unsigned int printf_string(char *message, unsigned int line, unsigned int column) // the message and then the line #
{
        char *vidmem = (char *) VIDMEM;
        unsigned int i = 0;
        int initial_p = 0;

        i = ((line*80) + column)*2;
        initial_p = i;
		
        while(*message != '\0')
        {
                if(*message =='\n') // check for a new line
                {
                        line++;
                        i = (line*80*2);
                        message++;
                } else {
                        vidmem[i] = *message;
                        message++;
                        i++;
                        vidmem[i] = WHITE_TXT;
                        i++;
                }
        }
        vidmem[i] = '\0';
        i++;
        vidmem[i] = WHITE_TXT;
        i++;

        return((int)(i - initial_p)/2);
}

uint64_t stoi(char *s) // the message and then the line #
{
	uint64_t i;
	i = 0;
	while(*s >= '0' && *s <= '9')
	{
		i = i * 10 + (*s - '0');
		s++;
	}
	return i;
}


uint64_t octalToDecimal(uint64_t n)
{

	uint64_t decimal_equiv = 0, i = 0, rem = 0, pow_oct = 1;
	while (n!=0)
	{
		rem = (int)n%10;
		n /= 10;
		if(i>0)			
			pow_oct *=8;
		decimal_equiv += rem*pow_oct;
		i++;
	}
	return decimal_equiv;
}

/*
int strcmp(char *str1, char *str2)
{
    if (*str1 < *str2)
        return -1;
 
    if (*str1 > *str2)
        return 1;
 
    if (*str1 == '\0')
        return 0;
 
    return strcmp(str1 + 1, str2 + 1);
}
*/

// will spit out the integer at the given line number
unsigned int printf_int(int message, unsigned int line, unsigned int column) // the message and the line #
{
        char *vidmem = (char *) VIDMEM;
        unsigned int i = 0;

        int digits[12];
        unsigned int remainder = 0;
        unsigned int quotient = message;
        int cnt = 0;
        int initial_p = 0;

        i = ((line*80) + column)*2;
        initial_p = i;
        int_array_reset(digits, 12);

        while(quotient >= 10) {
                remainder = (int) (quotient % 10);
                quotient = (int) (quotient / 10);

                digits[cnt] = remainder;
                cnt++;
        }
        digits[cnt] = quotient;
        while(cnt >= 0) {
              vidmem[i] = digits[cnt] + 48;   
                cnt--;
                i++;
                vidmem[i] = WHITE_TXT;
                i++;  
        }
        return((int)(i - initial_p)/2);
}

// will spit out the integer at the given line number
unsigned int printf_integer(int message, unsigned int line, unsigned int column, char* str) // the message and the line #
{
//        char *vidmem = (char *) VIDMEM;
        unsigned int i = 0;

        int digits[12];
//      char char_digits[12];
        unsigned int remainder = 0;
        unsigned int quotient = message;
        int cnt = 0;
        int initial_p = 0;
//      int j = 0;

        i = ((line*80) + column)*2;
        initial_p = i;
        // int_array_reset(digits, 12);

        while(quotient >= 10) {
                remainder = (int) (quotient % 10);
                quotient = (int) (quotient / 10);

                digits[cnt] = remainder;
                cnt++;
        }
        digits[cnt] = quotient;
        while(cnt >= 0) {
                *str++ = digits[cnt] + 48;
//              vidmem[i] = digits[cnt] + 48;   
//              char_digits[j++] = digits[cnt];
                cnt--;
//                i++;
//                vidmem[i] = WHITE_TXT;
//                i++;  
        }
        *str++ = '\0';
        return((int)(i - initial_p)/2);
//      return char_digits;
}

// will spit out the hexadecimal at the given line number
unsigned int printf_hexadecimal(unsigned long message, unsigned int line, unsigned int column, char* str) // the message and the line #
{
//        char *vidmem = (char *) VIDMEM;
        unsigned int i = 0;

        int cnt = 0;
        int initial_p = 0;
        char result[24];
        char hex[] = "0123456789abcdef";
        unsigned long quotient = message;

        i = ((line*80) + column)*2;
        initial_p = i;
        // char_array_reset(result, 8);

        while(quotient > 0) {
                result[cnt++] = hex[(quotient % 16)];
                quotient = (quotient / 16);
        }
//      vidmem[i++] = '0';
//      vidmem[i++] = WHITE_TXT;
//      vidmem[i++] = 'x';
//      vidmem[i++] = WHITE_TXT;        

        *str++ = '0';
        *str++ = 'x';

        cnt--;
        while(cnt >= 0)
        {
                *str++ = result[cnt];
//              vidmem[i++] = result[cnt];
//              vidmem[i++] = WHITE_TXT;
                cnt--;
        }

        *str++ = '\0';
        return((int)(i - initial_p)/2);
}

// Will show the current cursor position with an under score
void show_cursor(signed int memory_used)
{

	if((cursor_p_x + memory_used) > 79)
	{
                cursor_p_x = ((cursor_p_x + memory_used) - 80);
                //TODO: Add the code for scrolling if cursor_p_y > 24
                // call start_scroller()
		if(cursor_p_y == 23) 
		{
			start_scroller();
		}
		else 
		{
                	cursor_p_y++;
		}
        }
	else if((cursor_p_x + memory_used) < 0)
	{
		cursor_p_x = (80 + memory_used);
		cursor_p_y--;
	}
	else
        {
                cursor_p_x = cursor_p_x + memory_used;
        }
        printf_char('_', cursor_p_y, cursor_p_x);
}

// Will clear the screen with while on black text
void clear_screen() // clear the entire text screen
{
        char *vidmem = (char *) VIDMEM;
        unsigned int i = 0;
        while(i < (80*25*2))
        {
                vidmem[i] = ' ';
                i++;
                vidmem[i] = WHITE_TXT;
                i++;
        }
	//printf("%s", "\t\t\tWelcome to SBUnix....!!\n");
	show_cursor(0);
}

// Will handle the backspace key
void handle_backspace()
{
	char *vidmem = (char *) VIDMEM;	
	unsigned int i = ((cursor_p_y * 80) + cursor_p_x) * 2;
		
	vidmem[i++] = ' ';
	vidmem[i] = WHITE_TXT;
	
	cursor_p_x--;
        printf_char('_', cursor_p_y, cursor_p_x);
	show_cursor(-1);
}

// Will handle the enter'\n' character
void handle_enter() 
{
	char *vidmem = (char *) VIDMEM;
        unsigned int i = ((cursor_p_y * 80) + cursor_p_x) * 2;

        vidmem[i++] = ' ';
        vidmem[i] = WHITE_TXT;

	if(cursor_p_y == 23) 
	{
		start_scroller();
	}
	else 
	{
		cursor_p_x = 0;
		cursor_p_y++;
	}
	show_cursor(0);
}

// Printf method with support for variable arguments
int kprintf(const char *fmt, ...)
{
        va_list args;

        int len = 0;
        int str_len = 0;
        int cnt = 0;
        char str[1024];
        char str_temp[1024];
	int i = 0;

        va_start(args, fmt);

        // flush array
        // char_array_reset(str, 1024);
        // char_array_reset(str_temp, 1024);
	// memset(str, 0, 1024);
	// memset(str_temp, 0, 1024);
	
        // TODO: write the code to print        
        for(;*fmt;)
        {
                if(*fmt != '%') {
                        str[cnt++] = *fmt++;
                        continue;
                }

                fmt++;
                switch(*fmt)
                {
                        case 'c':
                                str[cnt++] = (unsigned char) va_arg(args, int);
                                fmt++;
                                continue;
                        case 'd':
				i = 0;
                                printf_integer(va_arg(args, int), cursor_p_y, cursor_p_x, str_temp);
                                len = strlen(str_temp);
                                while(i < len)
				{
                                        str[cnt++] = str_temp[i++];
                                }
                                fmt++;
                                continue;
                        case 's':
				i = 0;
                                str_temp = va_arg(args, char [1024]);
                                len = strlen(str_temp);
                                while(i < len)
                                {
                                        str[cnt++] = str_temp[i++];
                                }
                                fmt++;
                                continue;
                        case 'x':
				i = 0;
                                printf_hexadecimal(va_arg(args, int), cursor_p_y, cursor_p_x, str_temp);
                                len = strlen(str_temp);
                                while(i < len)
                                {
                                        str[cnt++] = str_temp[i++];
                                }
                                fmt++;
                                continue;
                        case 'p':
				i = 0;
                                printf_hexadecimal(va_arg(args, unsigned long), cursor_p_y, cursor_p_x, str_temp);
                                len = strlen(str_temp);
                                while(i < len)
                                {
                                        str[cnt++] = str_temp[i++];
                                }
                                fmt++;
                                continue;
			default:
                                fmt++;
                                continue;
                }
        }
        str[cnt] = '\0';

        va_end(args);

        str_len = printf_string(str, cursor_p_y, cursor_p_x);
        show_cursor(str_len - 1);

        return (1);
}

