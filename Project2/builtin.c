#include "builtin.h"
#include <assert.h>
#include <ctype.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <linux/limits.h>
#include <pwd.h>
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
static void ls(char **args, int argcp);
static void cp(char **args, int argcp);
static void env(char **args, int argcp);

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
    } else if (strcmp(args[i], "cp") == 0) {
      cp(args, argcp);
      result = 1;
    } else if (strcmp(args[i], "env") == 0) {
      env(args, argcp);
      result = 1;
    } else if (strcmp(args[i], "ls") == 0) {
      ls(args, argcp);
      result = 1;
    }
  }

  return result;
}

static void exitProgram(char **args, int argcp) {
  int exit_val = 0;
  if (argcp >= 2) {
    exit_val = atoi(args[1]);
  }
  exit(exit_val);
}

static void pwd(char **args, int argcp) {
  // printf("running pwd\n");
  char *buffer = malloc(PATH_MAX * sizeof(char));
  char *path;
  if ((path = getcwd(buffer, PATH_MAX)) == NULL) {
    fprintf(stderr, "failed to get wd\n");
  }
  printf("%s\n", path);

  free(buffer);
}

static void cd(char **args, int argcp) {
  // printf("running cd\n");
  int uid = getuid();
  struct passwd *password = getpwuid(uid);
  const char *home = password->pw_dir;
  // Use getenv to find
  if (argcp < 2) {
    if (chdir(home) == -1) {
      fprintf(stderr, "invalid path\n");
    }
    return;
  }

  if (chdir(args[1]) == -1) {
    fprintf(stderr, "invalid path\n");
  }
  printf("cd\n");
}

static void ls(char **args, int argcp) {
  printf("ls\n");
  int uid = getuid();
  struct passwd *password = getpwuid(uid);
  char *buffer = malloc(PATH_MAX * sizeof(char));
  char *path;
  if ((path = getcwd(buffer, PATH_MAX)) == NULL) {
    fprintf(stderr, "failed to get wd\n");
    exit(-1);
  }

  DIR *dir;
  if ((dir = opendir(path)) == NULL) {
    fprintf(stderr, "failed to open dir\n");
    exit(-1);
  }

  struct dirent *dirent;
  while ((dirent = readdir(dir)) != NULL) {
    printf("%s\n", dirent->d_name);
  }
  closedir(dir);

  free(buffer);
}

static void cp(char **args, int argcp) {}

static void env(char **args, int argcp) {}
