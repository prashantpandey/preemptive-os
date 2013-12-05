#include <stdlib.h>
#include <stdio.h>

char* current_dir = "bin/";

int strlen(const char *s) {
        int n;

        for (n = 0; *s != '\0'; s++)
                n++;
        return n;
}


char* strcpy(char *dst, const char *src) {
        char *ret;

        ret = dst;
        while ((*dst++ = *src++) != '\0')
                /* do nothing */;
        return ret;
}

int strcmp(const char *p, const char *q) {
        while (*p || *q) {
                if (*p != *q)
                        return -1;
                p++, q++;
        }
        return 0;
}

char* strcat(char *dst, const char *src) {
        int len = strlen(dst);
        strcpy(dst + len, src);
        return dst;
}

void printStr(char* a)
{
	while(*a!='\0'){
                printf("%c",*a);
                a++;
        }
}

int main(int argc, char* argv[], char* envp[]) {
	int i = 0;
        int c = 0;
        //int j = 0;
        int temp_count = 0;
        //char temp_str[20];
        char tokens[5][20];
        //char buffer_copy[512];

        char buffer[512];
        int loop = 1;
        while(loop == 1)
        {
                /*
                path = getenv("MYPATH");
                if(path == NULL)
                      path = "/bin#.";
                printf("($MYPATH is %s)\n", path);
                */
                printf("\n[myshell:~ $ ] ");
                scanf((char*)buffer);

                i = 0;
                c = 0;
                temp_count = 0;

                while(buffer[i]!='\0')
                {
                        if(buffer[i]!=' ')
                        {
                                tokens[c][temp_count++] = buffer[i];
                        }
                        else
                        {
                                tokens[c++][temp_count] = '\0';
                                temp_count=0;
                        }
                        i++;
                }
		tokens[c][temp_count] = '\0';

                if (strcmp(tokens[0],"ls") == 0)
                {
                        readdir(current_dir);
                }
                else if (strcmp(tokens[0],"cd") == 0 && c!=0)
                {
                        strcat(current_dir, tokens[1]);
                        printStr(current_dir);
                }
                else if (strcmp(tokens[0], "pwd") == 0)
                {
                        printStr(current_dir);
                }
                i = 0;
                for (i=0; i<512; i++)
                {
                        buffer[i]='\0';
                }
                // printf("Command: %s", command);

                if(strcmp(tokens[0],"exit") == 0 || strcmp(tokens[0], "quit") == 0)
                {
                        loop = 0;
                        printf("Program Terminated\n");
                }
              	// parse(command, path);
        }
        return 0;
}


