#include <stdlib.h>
#include <stdio.h>

int main(int argc, char* argv[], char* envp[]) {
	
	/*
	volatile char * video_memory =  (volatile char *) (0xFFFFFFFF800B8000);
        *video_memory = '1';
        video_memory += 1;
        *video_memory = 0x0f;
        video_memory += 1;
 
        *video_memory = '2';
        video_memory += 1;
        *video_memory = 0x0f;
        video_memory += 1;
	*/
/*	
	printf("Hello World..!!\n");
	printf("Hello World..!!\n");
	
	char* buffer = (char* )malloc(512);
	scanf(buffer);
	int i = 0;
	while(buffer[i] != '\0'){
		printf("%c", buffer[i]);
		i++;
	}
	int id = getpid();
	printf("\nPid: %d", id);
*/	
	// uint64_t addr = opendir("bin/hello/");
        // printf("\n%p", addr);
        readdir("bin/");
        
	/*
	addr = open("bin/hello");
        printf("\nAddress of hello \n%p", addr);
        
	char buf[512];
        int size = read(open("bin/hello/hello.c"), 32, (uint64_t) buf);
        printf("\nSize of hello.c %d", size);
	*/
	while(1);
	return 0;
}




