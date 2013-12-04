#include <stdio.h>
#include <print.h>
#include <string.h>

//int main(int argc, char *argv[])
int showShell()
{
        /*
        char *command;
        char *path;
        char buffer[1024];
        command = (char *)malloc(sizeof(char));
        path = (char *)malloc(sizeof(char));
        */
        
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

		kprintf("myshell$ ");
                kscanf(buffer);
                //command = fgets(buffer, 1024, stdin);
                kprintf("Buffer: %s\n", buffer);
		int i = 0;
		for (i=0; i<512; i++)
		{
			buffer[i]='\0';
		}		
                //printf("Command: %s", command);

                /*
                if(strcmp(command,"exit\n") == 0 || strcmp(command, "quit\n") == 0)
                {
                        loop = 0;
                        printf("Program Terminated\n");
                }
                 parse(command, path);
                */
        }
        return 0;
}
