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
    fprintf(stderr, "At least one flag is required.\n");
    usage();
    exit(EXIT_FAILURE);
  }

  int file_len = 0;
  if (mode == ENCRYPT) {
    file_len = process(input_file_name, ENCRYPT);
  } else {
    file_len = process(input_file_name, DECRYPT);
  }
  // file name will not exceed 128 characters.

  
  return 0;
}

// Encrypt or decrypt file based n mode. Return 
// input file length
int process(char *file_name, enum mode mode) {
  struct timeval time;
  struct timezone timezone;
  int success = gettimeofday(&time, &timezone);
  if (success == -1) {
    fprintf(stderr, "failed getting time");
    exit(EXIT_FAILURE);
  }
  time_t start_ms = time.tv_usec;
  // printf("start time: %ld\n", time.tv_sec);

  int input_fd = open(file_name, O_RDONLY);
  if (input_fd == -1) {
    fprintf(stderr, "Failed to open file for reading\n");
    exit(EXIT_FAILURE);
  }

  off_t input_file_length = lseek(input_fd, 0, SEEK_END);
  printf("Input file length: %ld\n", input_file_length);
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
  int output = open(output_filename, O_RDWR | O_CREAT, S_IRWXU);
  int written = write(output, buffer, input_file_length);
  if (written == -1) {
    fprintf(stderr, "Failed to write to output file/n");
    exit(EXIT_FAILURE);
  }

  free(buffer);

  // Print file info
  success = gettimeofday(&time, &timezone);
  if (success == -1) {
    fprintf(stderr, "failed getting time");
    exit(EXIT_FAILURE);
  }
  time_t end_ms = time.tv_usec;
  printf("time elapsed: %lu\n", end_ms - start_ms);
  // Write values to output file
  int fd = open("output.txt", O_APPEND | O_WRONLY, S_IRWXU);
  char strang[30];
  sprintf(strang, "%d %lu\n", file_len, end_ms - start_ms); 
  // printf("%s", strang);
  write(fd, strang, strlen(strang));
  return input_file_length;
}

int decrypt(char *file_name) { return 0; }

void usage(void) {
  fprintf(stderr, "Usage: filesec -e|-d [filename] \n");
  exit(EXIT_FAILURE);
}
