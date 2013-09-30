#ifndef _STDIO_H
#define _STDIO_H

//#include <unistd.h>

int printf(const char *format, ...);
int scanf(const char *format, ...);
unsigned int printf_string(char *message, unsigned int line, unsigned int column);
unsigned int printf_int(int message, unsigned int line, unsigned int column);
unsigned int printf_char(char message, unsigned int line, unsigned int column);
void clear_screen();
void clear_line(unsigned int cursor_p_y, unsigned int cursor_p_x);
void handle_backspace();
void handle_enter();
#endif
