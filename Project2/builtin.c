#include "builtin.h"
#include <assert.h>
#include <ctype.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <grp.h>
#include <linux/limits.h>
#include <pwd.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>
#define BUFFER_SIZE 400

// Prototypes
static void exitProgram(char **args, int argcp);
static void cd(char **args, int argpcp);
static void pwd(char **args, int argcp);
static void ls(char **args, int argcp);
static void cp(char **args, int argcp);
static void env(char **args, int argcp);
static int print_long(char *fullpath, struct dirent *dirent);

/* builtIn
 ** built in checks each built in command the given command, if the given
 *command matches one of the built in commands, that command is called and
 *builtin returns 1. If none of the built in commands match the wanted command,
 *builtin returns 0;
 */
int builtIn(char **args, int argcp) {
  int result = 0;
  // write your code
  if (strcmp(args[0], "exit") == 0) {
    exitProgram(args, argcp);
    result = 1;
  } else if (strcmp(args[0], "pwd") == 0) {
    pwd(args, argcp);
    result = 1;
  } else if (strcmp(args[0], "cd") == 0) {
    cd(args, argcp);
    result = 1;
  } else if (strcmp(args[0], "cp") == 0) {
    cp(args, argcp);
    result = 1;
  } else if (strcmp(args[0], "env") == 0) {
    env(args, argcp);
    result = 1;
  } else if (strcmp(args[0], "ls") == 0) {
    ls(args, argcp);
    result = 1;
  } else {
    result = 0;
  }

  return result;
}

// Global variables used to break while loop in main
bool exiting;
int exit_value;

// TODO: Make sure everything gets freed before exit is called
static void exitProgram(char **args, int argcp) {
  int exit_val = 0;

  if (argcp >= 2) {
    exit_val = atoi(args[1]);
  }

  exiting = true;
  exit_value = exit_val;
}

// Print the path of the current working directory
static void pwd(char **args, int argcp) {
  char *buffer = malloc(PATH_MAX * sizeof(char));

  char *path = getcwd(buffer, PATH_MAX);
  if (path == NULL) {
    perror("failed to get wd\n");
  }

  printf("%s\n", path);

  free(buffer);
}

// Change directory
static void cd(char **args, int argcp) {
  int uid = getuid();
  struct passwd *password = getpwuid(uid);
  if (password == NULL) {
    perror("failed to get home directory");
    return;
  }
  const char *home = password->pw_dir;

  // Use getenv to find
  if (argcp < 2) {
    if (chdir(home) == -1) {
      perror("invalid path\n");
    }
    return;
  }

  if (chdir(args[1]) == -1) {
    perror("invalid path\n");
  }
}

// List information about current directory
static void ls(char **args, int argcp) {
  // Boolean for -l
  bool long_format = false;

  // Handle command line arguments
  if (argcp >= 2) {
    if (strcmp(args[1], "-l") != 0) {
      fprintf(stderr, "usage: ls [-l]\n");
      return;
    }
    long_format = true;
  }

  // Create buffer for path
  char *current_path = malloc(PATH_MAX * sizeof(char));
  if (current_path == NULL) {
    perror("Malloc failed");
    free(current_path);
    return;
  }

  // Populate path
  char *path = getcwd(current_path, PATH_MAX);
  if (path == NULL) {
    perror("malloc failed");
    free(current_path);
    return;
  }

  // Open directory
  DIR *dir = opendir(path);
  if (dir == NULL) {
    perror("Opendir failed");
    free(current_path);
    return;
  }

  // loop through directory entries and print contents
  struct dirent *dirent = readdir(dir);
  while (dirent != NULL) {
    if (long_format) {
      print_long(current_path, dirent);
    } 
    printf(" %s\n", dirent->d_name);

    dirent = readdir(dir);
  }

  closedir(dir);
  free(current_path);
}

// Print the long version of dirent for ls
// returns -1 on error
static int print_long(char *path, struct dirent *dirent) {
  char *fullpath = calloc(1, BUFFER_SIZE);
  struct stat statp;
  sprintf(fullpath, "%s%s%s", path, "/", dirent->d_name);

  int stat_result = stat(fullpath, &statp);

  if (stat_result == -1) {
    perror("stat call failed\n");
    free(fullpath);
    return -1;
  }

  // File permissions
  printf((statp.st_mode & S_IFDIR) ? "d" : "-");
  printf((statp.st_mode & S_IRUSR) ? "r" : "-");
  printf((statp.st_mode & S_IWUSR) ? "w" : "-");
  printf((statp.st_mode & S_IXUSR) ? "x" : "-");
  printf((statp.st_mode & S_IRGRP) ? "r" : "-");
  printf((statp.st_mode & S_IWGRP) ? "w" : "-");
  printf((statp.st_mode & S_IXGRP) ? "x" : "-");
  printf((statp.st_mode & S_IROTH) ? "r" : "-");
  printf((statp.st_mode & S_IWOTH) ? "w" : "-");
  printf((statp.st_mode & S_IXOTH) ? "x" : "-");

  // Number of links
  printf(" %2ld", (long)statp.st_nlink);

  // Owner name
  struct passwd *pw = getpwuid(statp.st_uid);
  if (pw != NULL)
    printf(" %s", pw->pw_name);
  else
    printf(" %d", statp.st_uid);

  // Populate group struct
  struct group *gr = getgrgid(statp.st_gid);
  if (gr != NULL) {
    printf(" %s", gr->gr_name);
  }

  // File size
  printf(" %5ld", (long)statp.st_size);

  // Last modified time
  char timebuf[80];
  strftime(timebuf, sizeof(timebuf), "%b %d %H:%M", localtime(&statp.st_mtime));
  printf(" %s", timebuf);

  free(fullpath);
  return 0;
}

// Copty the contentx of source file to destination
static void cp(char **args, int argcp) {
  struct stat fs;
  int r;

  // handle command line args
  if (argcp < 3) {
    printf("Usage: cp  <src_file_name target_file_name>\n");
  }

  // Populate stat with file permissions
  r = stat(args[1], &fs);
  struct stat file_stat;
  int stat_result = stat(args[1], &file_stat);
  if (stat_result) {
    perror("Error getting file permissions");
    return;
  }

  // Open original file
  int input_fd = open(args[1], O_RDONLY);
  if (input_fd == -1) {
    perror("Error creating new file");
    return;
  }

  // Open the new file
  int output_fd = open(args[2], O_CREAT | O_WRONLY | O_TRUNC);
  if (output_fd == -1) {
    perror("Error creating new file");
    return;
  }

  // Set the permissions of the new file to match the old
  if (fchmod(output_fd, file_stat.st_mode) == -1) {
    perror("Error setting file permissions");
    return;
  }

  // Copy the contents of input buffer to output buffer
  char *buffer = malloc(BUFFER_SIZE * sizeof(char));
  int bytes_read = read(input_fd, buffer, BUFFER_SIZE);
  while (bytes_read > 0) {
    int bytes_written = write(output_fd, buffer, bytes_read);
    if (bytes_written < bytes_read) {
      perror("failed to write to output file\n");
      return;
    }
    bytes_read = read(input_fd, buffer, BUFFER_SIZE);
  }

  // Close the files
  close(output_fd);
  close(input_fd);
  free(buffer);
}

// Print all environment variables, or set with: env [KEY=Value]
static void env(char **args, int argcp) {
  // Change environment variable if argument added
  if (argcp == 2) {
    if (putenv(args[1]) != 0) {
      perror("Failure to modify environment variable\n");
      printf("Usage = env [KEY=Value]\n");
      return;
    }
  }

  // Print all env variables, iterate through using env_index pointer
  extern char **environ;
  char **var_index = environ;
  while (*var_index != NULL) {
    printf("%s\n", *var_index);
    var_index++;
  }
}
