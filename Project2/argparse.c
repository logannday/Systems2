#include "argparse.h"
#include <ctype.h>
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#define FALSE (0)
#define TRUE (1)

/*
 * argCount is a helper function that takes in a String and returns the number
 * of "words" in the string assuming that whitespace is the only possible
 * delimeter.
 */
static int argCount(char *line) {
  // write your code
  bool in_whitespace = true;
  int count = 0;
  char *tmp = line;

  while(*tmp != '\0') {
    // Count words from the beginning
    if (!isspace(*tmp) && in_whitespace) {
      count++;
      in_whitespace = false;
    } else if (isspace(*tmp)) {
      in_whitespace = true;
    }
    tmp++;
  }

  return count;
}

/*
 *
 * Argparse takes in a String and returns an array of strings from the input.
 * The arguments in the String are broken up by whitespaces in the string.
 * The count of how many arguments there are is saved in the argcp pointer
 */
char **argparse(char *line, int *argcp) {
  // Iterate through the string
  *argcp = argCount(line);
  // Todo: free this
  // Initialize all pointers to null
  char **strings = (char**)calloc(*argcp, sizeof(char*));
  for (int i = 0; i < *argcp; i++) {
    // Maybe don't hardcode this
    strings[i] = malloc(30 * sizeof(char));
  }


  return NULL;
}
