#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <signal.h>


int main(int argc, char **argv){
	int i;
	int index = 0;
	int numChildren = 6;
	pid_t pids[numChildren];
	pid_t pid;
	for (i = 0; i < 3; i++){
		pids[index++] = fork();
		if (pids[index-1] == 0){
			execl("./consumer", "./consumer", NULL);
			
		}
		sleep(1);
	}
	for (i = 0; i < 3; i++){
		pids[index++] = fork();
		if (pids[index-1] == 0){
			execl("./producer", "./producer", NULL);
			
		}
		sleep(1);
	}
	printf("Hit any key to kill all children\n");
	getchar();
	for (i = 0; i < numChildren; i++){
		printf("Killing process %d\n", pids[i]);
		kill(pids[i], SIGINT);
	}
		
}
