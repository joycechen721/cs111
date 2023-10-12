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
		exit(EXIT_FAILURE);
	}

	int proc_num = 1;
	int status = 0;
	
	//iterate through each process, except for the last process.
    for (; proc_num < argc - 1; proc_num++){
		//create pipe
		int fds[2];
		pipe(fds);

		//fork the program
        int child_pid = fork();

		//child -- process that'll execute
        if (child_pid == 0) {
			close(fds[0]);
			dup2(fds[1], 1);
			//handle invalid programs
			if (execlp(argv[proc_num], argv[proc_num], NULL) == -1) {
				exit(EXIT_FAILURE);
   			}
        }

		//parent -- waits for child process, sets stdin to read pipe for next execution round
		else if (child_pid > 0) {
			waitpid(child_pid, &status, 0);
			//check whether child exited with error
			int exit_status = WEXITSTATUS(status);
			if(exit_status != 0) {
				exit(exit_status);
			}
			dup2(fds[0], 0);
			close(fds[1]);
        }

		//failed fork()
		else {
			exit(EXIT_FAILURE);
		}
    }

	//execute the last instruction
	if (execlp(argv[proc_num], argv[proc_num], NULL) == -1) {
		exit(EXIT_FAILURE);
	}

	return 0;
}