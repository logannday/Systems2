/* CS 347 -- Mini Shell!
 *
 * Original author Phil Nelson 2000
 */
#include "argparse.h"
#include "builtin.h"
#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdbool.h>

#define DEFAULT_BUFFER_SIZE 20

/* PROTOTYPES */
void processline(char *line);
ssize_t getinput(char **line, size_t *size);
void execute(char **args, int argcp); 
/* main
 * This function is the main entry point to the program.  This is essentially
 * the primary read-eval-print loop of the command interpreter.
 */
int main() {
  extern bool exiting;          // Variables declared in builtins.c, called by exit()
  extern int exit_value;

  // Initialize line used to get input
  char **line = malloc(5 * sizeof(char*));
  *line = malloc(DEFAULT_BUFFER_SIZE * sizeof(char));
  size_t size = DEFAULT_BUFFER_SIZE * sizeof(char);

  // Process commands until exit() is run
  while (!exiting) {
    size_t result = getinput(line, &size);
    processline(*line);
  }

  free(*line);
  free(line);
  return EXIT_SUCCESS;
}

/* getinput
 * line     A pointer to a char* that points at a buffer of size *size or NULL.
 * size     The size of the buffer *line or 0 if *line is NULL.
 * returns  The length of the string stored in *line.
 *
 * This function prompts the user for input, e.g., %myshell%.  If the input fits
 * in the buffer pointed to by *line, the input is placed in *line.  However, if
 * there is not enough room in *line, *line is freed and a new buffer of
 * adequate space is allocated.  The number of bytes allocated is stored in
 * *size. Hint: There is a standard i/o function that can make getinput easier
 * than it sounds.
 */
ssize_t getinput(char **line, size_t *size) {
  printf("%% ");
  fflush(stdout);
  ssize_t chars_read = getline(line, size, stdin);

  return chars_read;
}

void execute(char **args, int argcp) {
  pid_t child_pid = 0;
  char command[150];
  if ((child_pid = fork()) == 0) {
    // Create string of path
    sprintf(command, "%s%s", "/bin/", args[0]);

    // execl the command, pass in pointer to args+1, make sure that the array is
    // null terminated
    if (execv(command, args) == -1) {
      printf("Arg: |%s|\n", args[1]);
      perror("failed to execute command\n");
    }
  } else {
    // wait for child
    child_pid = wait(NULL);
  }
}

/* processline
 * The parameter line is interpreted as a command name.  This function creates a
 * new process that executes that command.
 * Note the three cases of the switch: fork failed, fork succeeded and this is
 * the child, fork succeeded and this is the parent (see fork(2)).
 * processline only forks when the line is not empty, and the line is not trying
 * to run a built in command
 */
void processline(char *line) {
  pid_t cpid;
  int status;
  int argCount;
  char **arguments = argparse(line, &argCount);

  int ran_builtin = builtIn(arguments, argCount);
  int command_length = strlen(*arguments);

  if (ran_builtin == 0 && command_length != 0) {
    execute(arguments, argCount);
  }

  // Free all of the strings in the array, then the array itself
  for (int i = 0; i < argCount; i++) {
    free(arguments[i]);
  }
  free(arguments);
}
