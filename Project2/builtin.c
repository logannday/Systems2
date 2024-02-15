#include "builtin.h"
#include <assert.h>
#include <ctype.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <linux/limits.h>
#include <pwd.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#define BUFFER_SIZE 128000

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

// bool exiting;
// int exit_value;

// TODO: Make sure everything gets freed before exit is called
static void exitProgram(char **args, int argcp) {
  int exit_val = 0;

  if (argcp >= 2) {
    exit_val = atoi(args[1]);
  }

  // exiting = true;
  // exit_value = exit_val;
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

// TODO: implement -l
static void ls(char **args, int argcp) {
  bool long_format = false;
  if (argcp >= 2) {
    if (strcmp(args[1], "-l") != 0) {
      fprintf(stderr, "usage: ls [-l]\n");
      return;
    }
    long_format == true;
  }

  int uid = getuid();
  struct passwd *password = getpwuid(uid);

  char *buffer;
  if ((buffer = malloc(PATH_MAX * sizeof(char))) == NULL) {
    fprintf(stderr, "Malloc failed\n");
    exit(-1);
  }

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
    // use lstat to grab info
  }
  closedir(dir);

  free(buffer);
}

// TODO: Make sure to copy permissions as well!!
static void cp(char **args, int argcp) {
  if (argcp < 3) {
    printf("Usage: cp  <src_file_name target_file_name>\n");
  }

  printf("%s %s\n", args[1], args[2]);

  FILE *input;
  if ((input = fopen(args[1], "r")) == NULL) {
    fprintf(stderr, "failed to open %s\n", args[1]);
    return;
  }

  FILE *output;
  if ((output = fopen(args[2], "w")) == NULL) {
    fprintf(stderr, "failed to open %s\n", args[2]);
    return;
  }

  char buffer[BUFFER_SIZE];
  size_t bytes_read;
  while ((bytes_read = fread(buffer, BUFFER_SIZE, sizeof(char), input)) != 0) {
    if ((fwrite(buffer, BUFFER_SIZE, sizeof(char), output)) != 0) {
      fprintf(stderr, "failed to write bytes\n");
      return;
    }
  }
}

// Print all environment variables, or set with: env [KEY=Value]
static void env(char **args, int argcp) {

  if (argcp == 2) {
    if (putenv(args[1]) != 0) {
      fprintf(stderr, "Failure to modify environment variable\n");
      fprintf(stderr, "Usage = env [KEY=Value]\n");
    }
  }

  extern char **environ;
  char** tmp = environ;
  while (*tmp != NULL) {
    printf("%s\n", *tmp);
    tmp++;
  }
}
