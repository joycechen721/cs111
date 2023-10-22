#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>

int main(int argc, char *argv[])
{
	//handle no arguments
	if (argc == 1) {
		errno = EINVAL;
		exit(errno);
	}

	int proc_num = 1;
	int status = 0;
	
	//iterate through each process, except for the last process.
    for (; proc_num < argc - 1; proc_num++){
		//create pipe
		int fds[2];
		if (pipe(fds) == -1){
			exit(errno);
		}
		//fork the program
        int child_pid = fork();

		//child -- process that'll execute
        if (child_pid == 0) {
			//close read-end
			if (close(fds[0]) == -1){
				exit(errno);
			}
			//set stdout to write-end of pipe
			if(dup2(fds[1], 1) < 0){
				exit(errno);
			}
			//handle invalid programs
			if (execlp(argv[proc_num], argv[proc_num], NULL) == -1) {
				exit(errno);
   			}
        }

		//parent -- waits for child, sets stdin to read pipe for next execution round
		else if (child_pid > 0) {
			waitpid(child_pid, &status, 0);
			//check whether child exited with error
			int exit_status = WEXITSTATUS(status);
			if(exit_status != 0) {
				exit(exit_status);
			}
			//set stdin to read-end of pipe
			if(dup2(fds[0], 0)){
				exit(errno);
			}
			//close write-end
			if(close(fds[1]) == -1){
				exit(errno);
			}
        }

		//failed fork()
		else {
			exit(errno);
		}
    }

	//execute the last instruction (stdout is default)
	if (execlp(argv[proc_num], argv[proc_num], NULL) == -1) {
		exit(errno);
	}

	return 0;
}
