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
	
	printf("\nProcess 1\n");
	printf("Hello World..!!\n");
	
	// Checking malloc system call
	char* buffer = (char* )malloc(512);
	scanf(buffer);
	int i = 0;
	while(buffer[i] != '\0'){
		printf("%c", buffer[i]);
		i++;
	}

	// Checking getpid system call
	int id = getpid();
	printf("\nPid: %d", id);

	// Checking opendir system call
	uint64_t addr = opendir("bin/");
        printf("\n%p", addr);

	// Checking closedir and readdir system call
	closedir(addr);
        readdir("bin/");
        
	// Checking open system call 
	addr = open("bin/hello");
        printf("\nAddress of hello \n%p", addr);
	
	// Checking close system call	
	close(addr);

	// Checking fork system call
	/*        
	char buf[512];
        int size = read(open("bin/hello/hello.c"), 32, (uint64_t) buf);
        printf("\nSize of hello.c %d", size);
	*/
	
	id = fork();
	if(id == 0) {
		printf("Child is here..!!");
	}	
	else {
		printf("Parent is here");
	}
	
	
	while(1);
	return 0;
}




