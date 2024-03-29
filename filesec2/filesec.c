#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <unistd.h>

enum mode { ENCRYPT, DECRYPT };

int process(char *file_name, enum mode mode);
void usage(void);

int main(int argc, char **argv) {

  int opt;
  enum mode mode = ENCRYPT;
  char input_file_name[128];

  // Handle command line arguments
  while ((opt = getopt(argc, argv, "e:d:")) != -1) {
    switch (opt) {
    case 'e':
      mode = ENCRYPT;
      strcpy(input_file_name, optarg);
      input_file_name[strlen(optarg)] = '\0';
      break;
    case 'd':
      mode = DECRYPT;
      strcpy(input_file_name, optarg);
      input_file_name[strlen(optarg)] = '\0';
      break;
    default:
      usage();
      exit(EXIT_FAILURE);
    }
  }

  // Check that at least one flag was passed
  if (optind == 1) {
    usage();
    exit(EXIT_FAILURE);
  }

  if (mode == ENCRYPT) {
    process(input_file_name, ENCRYPT);
  } else {
    process(input_file_name, DECRYPT);
  }

  return 0;
}

/* Encrypt or decrypt file based on mode paramater. Return
 * input file length
 */
int process(char *file_name, enum mode mode) {
  struct timeval time;
  struct timezone timezone;
  int success = gettimeofday(&time, &timezone);
  if (success == -1) {
    char message[] = "failed getting time";
    write(1, message, strlen(message));
    exit(EXIT_FAILURE);
  }
  time_t start_ms = time.tv_usec;

  int input_fd = open(file_name, O_RDONLY);
  if (input_fd == -1) {
    char message[] = "Failed to open file for reading\n";
    write(STDERR_FILENO, message, strlen(message));
    exit(EXIT_FAILURE);
  }

  off_t input_file_length = lseek(input_fd, 0, SEEK_END);
  lseek(input_fd, 0, SEEK_SET);
  char *buffer = (char *)malloc(input_file_length * sizeof(char));

  read(input_fd, buffer, input_file_length);

  for (int i = 0; i < input_file_length; i++) {
    if (mode == ENCRYPT) {
      buffer[i] += 100;
    } else {
      buffer[i] -= 100;
    }
  }

  // Creating output file name
  char *pointer_to_dot = strchr(file_name, '.');
  *pointer_to_dot = '\0'; // remove .txt extension from original filename
  char *suffix = (mode == ENCRYPT) ? "_enc.txt" : "_dec.txt";
  char *output_filename = strcat(file_name, suffix);

  // Writing buffer to output file
  int output_fd = open(output_filename, O_RDWR | O_CREAT, S_IRWXU);
  int written = write(output_fd, buffer, input_file_length);
  if (written == -1) {
    char message[] = "Failed to write to output file\n";
    write(STDERR_FILENO, message, strlen(message));
    exit(EXIT_FAILURE);
  }

  free(buffer);

  // Get time after encoding finishes
  success = gettimeofday(&time, &timezone);
  if (success == -1) {
    char message[] = "failed getting time";
    write(STDERR_FILENO, message, strlen(message));
    exit(EXIT_FAILURE);
  }
  time_t end_ms = time.tv_usec;

  // This code was used to create a log file of input sizes and runtimes
  int fd = open("log.txt", O_WRONLY | O_CREAT, S_IRWXU);
  lseek(fd, 0, SEEK_END);
  char strang[30];
  sprintf(strang, "%ld %lu\n", input_file_length, end_ms - start_ms);
  write(fd, strang, strlen(strang));
  close(fd);
  close(input_fd);
  close(output_fd);
  return input_file_length;
}

void usage(void) {
  char usage[] = "Usage: \n./filesec -e|-d [filename] \n";
  fflush(stdout);
  write(1, usage, strlen(usage));
  exit(EXIT_FAILURE);
}
