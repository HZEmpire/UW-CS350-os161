#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#define MAX_BUFSZ (512)
#define AVRSRV_INIT (10)
#define AV_MAXTOK (256)

char *outcmd;
int avsize;
char *av[AV_MAXTOK];

/*
 * Read in a line and parse it into
 * input tokens, which in this case 
 * are the program names and command
 * line arguments.
 */
void parseline(char *line)
{
	char *a;
	int n;

	for (n = 0; n < AV_MAXTOK; n++)
		av[n] = NULL;

	outcmd = NULL;
	a = strtok(line, " \t\r\n");
	for (n = 0; a; n++) {
		if (n >= AV_MAXTOK) {
			fprintf(stderr, "Too many tokens in input\n");
			exit(1);
		}

		/* If not a pipe we're still in the same command. */
		if (a[0] != '|') {
			av[n] = a;
			a = strtok(NULL, " \t\r\n");
			continue;
		}

		/* Otherwise this is a new command. */

		/* 
		 * If the token was just the pipe, 
		 * grab the rest of the input and put
		 * it into outcmd for further processing.
		 */
		if (!a[1]) {
			outcmd = strtok(NULL, "");
			break;
		} 

		/* 
		 * If the user did not put a space between
		 * the pipe and the next token we clip off
		 * the pipe character and fix up the string
		 * to replace any \0 characters added by the
		 * strtok() call with spaces.
		 */
		outcmd = a + 1;
		a = strtok(NULL, "");
		while (a > outcmd && !a[-1])
			*--a = ' ';

		/* Get the next token. */
		a = strtok(NULL, " \t\r\n");
	}
}

void doexec(void) {
	int fd;

	while (outcmd) {
		int pipefds[2];

		if (pipe(pipefds) < 0) {
			perror("pipe");
			exit(0);
		}

		switch (fork()) {
			case -1:
				perror("fork");
				exit(1);
			case 0:
				/* 
				 * XXX FILLMEIN(B): Replace the 
				 * new command's stdin.
				 */

				/* 
				 * The child must keep parsing the
				 * line to extract the next command
				 * in the pipeline.
				 */
				dup2(pipefds[0], 0);
				close(pipefds[0]);
				close(pipefds[1]);
				parseline(outcmd);
				break;
			default:
				/* 
				 * XXX FILLMEIN(B): Replace the 
				 * new command's stdout.
				 */

				/* 
				 * The parent only needs to execute
				 * the current command.
				 */
				dup2(pipefds[1], 1);
				close(pipefds[0]);
				close(pipefds[1]);
				outcmd = NULL;
				break;
		}
	}

	/*
         * XXX FILLMEIN(A): Execute the new function. Do
         * not forget to pass the argument vector to it.
         *
         * HINT: The command line arguments passed by
         * the user are found in char **av.
         */
	execvp(av[0], av);
	perror(av[0]);
	exit(1);
}

int main(void)
{
	char buf[MAX_BUFSZ];
	char *line;
	pid_t pid;

	for (;;) {
		/* Write out the command prompt and read the input. */	
    if (isatty(STDIN_FILENO)) {
      printf("$ ");
    }

		if (!(line = fgets(buf, sizeof(buf), stdin))) {
			exit(0);
		}

		/* Tokenize the input. */
		parseline(line);
		if (!av[0])
			continue;

		/*
                 * XXX FILLMEIN(A): Create a child process using fork().
                 * The parent should waits for the child, with waitpid(),
                 * while the child should call doexec(). The child never
                 * returns from doexec(), so you don't need to worry about
                 * handling errors.
                 */
		int rc = fork();
		if (rc < 0) {
			perror("fork");
			exit(1);
		} 
		
		else if (rc == 0) {
			doexec();
		} 
		
		else {
			waitpid(rc, NULL, 0);
		}
	}
}
