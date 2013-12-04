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
	
	printf("Hello World..!!\n");
	printf("Hello World..!!\n");
	
	char buffer[100];
	scanf(buffer);
	int i = 0;
	while(buffer[i] != '\0'){
		printf("%c", buffer[i]);
		i++;
	}
	int id = getpid();
	printf("Pid: %d", id);
	//uint64_t addr = (uint64_t)malloc(4096);
	//printf("%p", addr);
	//exit(1);
	//asm("int $0x80");
	while(1);
	return 0;
}




