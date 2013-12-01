#ifndef _STDIO_H
#define _STDIO_H

//#include <unistd.h>
#include <defs.h>

//int strcmp(char *str1, char *str2);
//char* strcpy(char *dst, const char *src);
// uint64_t stoi(char *s);
// uint64_t octalToDecimal(uint64_t n);
int printf(const char *format, ...);
int scanf(const char *format, ...);

// unsigned int kprintf_string(char *message, unsigned int line, unsigned int column);
// unsigned int kprintf_int(int message, unsigned int line, unsigned int column);
// unsigned int kprintf_char(char message, unsigned int line, unsigned int column);
// unsigned int strlen(const char *str);
// void clear_screen();
// void clear_line(unsigned int cursor_p_y, unsigned int cursor_p_x);
// void handle_backspace();
// void handle_enter();
#endif
