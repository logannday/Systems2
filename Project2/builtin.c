#include "builtin.h"
#include <assert.h>
#include <ctype.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <linux/limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

// Prototypes
static void exitProgram(char **args, int argcp);
static void cd(char **args, int argpcp);
static void pwd(char **args, int argcp);

/* builtIn
 ** built in checks each built in command the given command, if the given
 *command matches one of the built in commands, that command is called and
 *builtin returns 1. If none of the built in commands match the wanted command,
 *builtin returns 0;
 */
int builtIn(char **args, int argcp) {
  int result = 0;
  // write your code
  for (int i = 0; i < argcp; i++) {
    if (strcmp(args[i], "exit") == 0) {
      exitProgram(args, argcp);
      result = 1;
    } else if (strcmp(args[i], "pwd") == 0) {
      pwd(args, argcp);
      result = 1;
    } else if (strcmp(args[i], "cd") == 0) {
      cd(args, argcp);
      result = 1;
    }
  }
  return result;
}

static void exitProgram(char **args, int argcp) {
  int exit_val = 0;
  if (argcp == 2) {
    exit_val = atoi(args[1]);
  }
  exit(exit_val);
}

static void pwd(char **args, int argcp) {
  char *buffer = malloc(PATH_MAX * sizeof(char));
  char *path = getcwd(buffer, PATH_MAX);
  if (path == NULL) {
    fprintf(stderr, "invalid path");
  } else {
    printf("%s\n", buffer);
  }

  free(buffer);
}

static void cd(char **args, int argcp) {
  if (argcp < 2) {
    fprintf(stderr, "Usage: cd [directory name]\n");
    return;
  }
  int result = chdir(args[1]);
  if (result == -1) {
    fprintf(stderr, "invalid path");
  }
  printf("cd\n");
}
