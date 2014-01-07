#include <syscall.h>
#include <stdio.h>
#include <print.h>

#define va_start(v,l) __builtin_va_start(v,l)
#define va_arg(v,l)   __builtin_va_arg(v,l)
#define va_end(v)     __builtin_va_end(v)
//#define va_copy(d,s)  __builtin_va_copy(d,s)
typedef __builtin_va_list va_list;


// Will return the length of the string
unsigned int strlenp(const char *str)
{
        int i = 0;
        while(str[i] != '\0')
                i++;
        return i;
}

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

// will spit out the integer at the given line number
unsigned int printf_integer(int message, char* str) // the message and the line #
{
//        char *vidmem = (char *) VIDMEM;
        unsigned int i = 0;

        int digits[12];
//      char char_digits[12];
        unsigned int remainder = 0;
        unsigned int quotient = message;
        int cnt = 0;
        //int initial_p = 0;
//      int j = 0;

        int_array_reset(digits, 12);

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
                i++;
//                vidmem[i] = WHITE_TXT;
//                i++;  
        }
        *str++ = '\0';
        return i;
//      return char_digits;
}

// will spit out the hexadecimal at the given line number
unsigned int printf_hexadecimal(unsigned long message, char* str) // the message and the line #
{
//        char *vidmem = (char *) VIDMEM;
        unsigned int i = 0;

        int cnt = 0;
        // int initial_p = 0;
        char result[8];
        char hex[] = "0123456789abcdef";
        unsigned long quotient = message;

        char_array_reset(result, 8);

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
		i++;
        }

        *str++ = '\0';
        return i;
}

/*
* Handles print function
*/
int printf(const char *fmt, ...)
{
        va_list args;

        int len = 0;
        // int str_len = 0;
        int cnt = 0;
        char str[1024];
        char str_temp[1024];
	int i = 0;

        va_start(args, fmt);

        // flush array
        char_array_reset(str, 1024);
        char_array_reset(str_temp, 1024);

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
                                printf_integer(va_arg(args, int), str_temp);
                                len = strlenp(str_temp);
                                while(i < len)
				{
                                        str[cnt++] = str_temp[i++];
                                }
                                fmt++;
                                continue;
                        case 's':
				i = 0;
                                str_temp = va_arg(args, char [1024]);
                                len = strlenp(str_temp);
                                while(i < len)
                                {
                                        str[cnt++] = str_temp[i++];
                                }
                                fmt++;
                                continue;
                        case 'x':
				i = 0;
                                printf_hexadecimal(va_arg(args, int), str_temp);
                                len = strlenp(str_temp);
                                while(i < len)
                                {
                                        str[cnt++] = str_temp[i++];
                                }
                                fmt++;
                                continue;
                        case 'p':
				i = 0;
                                printf_hexadecimal(va_arg(args, unsigned long), str_temp);
                                len = strlenp(str_temp);
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
    __syscall1(2, (uint64_t)&str);
    return 0;
}
