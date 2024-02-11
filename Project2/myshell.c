/* CS 347 -- Mini Shell!
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

#define DEFAULT_BUFFER_SIZE 20
/* PROTOTYPES */

void processline(char *line);
ssize_t getinput(char **line, size_t *size);

/* main
 * This function is the main entry point to the program.  This is essentially
 * the primary read-eval-print loop of the command interpreter.
 */

int main() {
  char **line;
  size_t size = DEFAULT_BUFFER_SIZE * sizeof(char);
  line = (char **)malloc(sizeof(char *));
  *line = (char *)malloc(size);

  while (1) {
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
  if (line == NULL) {
    line = malloc(DEFAULT_BUFFER_SIZE * sizeof(char));
    *size = DEFAULT_BUFFER_SIZE;
  }

  int buffer_size = *size;
  int c;
  char *tmp = *line;
  int index = 0;
  // Read in chars from stdin, once the chars exceed
  while ((c = getc(stdin)) != '\n' && c != EOF) {
    if (index > buffer_size - 2) {
      buffer_size += 1;
      if ((realloc(*line, buffer_size)) == NULL) {
        fprintf(stderr, "failure\n");
        exit(1);
      }
    }
    *tmp++ = (char)c;
    index++;
  }
  *tmp = '\0';

  return buffer_size;
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
  /*check whether line is empty*/
  // write your code

  pid_t cpid;
  int status;
  int argCount;
  char **arguments = argparse(line, &argCount);
  // printf("argcount: %d\n", argCount);
  for (int i = 0; i < argCount; i++) {
    // printf("arg %d: %s\n", i, arguments[i]);
  }

  builtIn(arguments, argCount);

  // Free all of the strings in the array, then the array
  for (int i = 0; i < argCount; i++) {
    // printf("arguments[%d]: |%s|\n", i, arguments[i]);
    free(arguments[i]);
  }

  free(arguments);
}
