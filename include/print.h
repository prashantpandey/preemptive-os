#ifndef _PRINT_H
#define _PRINT_H

//#include <unistd.h>
#include <defs.h>

//int strcmp(char *str1, char *str2);
//char* strcpy(char *dst, const char *src);
uint64_t stoi(char *s);
uint64_t octalToDecimal(uint64_t n);

int kprintf(const char *format, ...);
int kscanf(const char *format, void* var);

unsigned int printf_string(char *message, unsigned int line, unsigned int column);
unsigned int printf_int(int message, unsigned int line, unsigned int column);
unsigned int printf_char(char message, unsigned int line, unsigned int column);
//unsigned int strlen(const char *str);
void clear_screen();
void clear_line(unsigned int cursor_p_y, unsigned int cursor_p_x);
void handle_backspace();
void handle_enter();
#endif
