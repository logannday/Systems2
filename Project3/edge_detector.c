#include <math.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>

#define LAPLACIAN_THREADS 4

/* Laplacian filter is 3 by 3 */
#define FILTER_WIDTH 3
#define FILTER_HEIGHT 3

#define RGB_COMPONENT_COLOR 255

typedef struct {
  unsigned char r, g, b;
} PPMPixel;

struct parameter {
  PPMPixel *image;         // original image pixel data
  PPMPixel *result;        // filtered image pixel data
  unsigned long int w;     // width of image
  unsigned long int h;     // height of image
  unsigned long int start; // starting point of work
  unsigned long int size;  // equal share of work (almost equal if odd)
};

struct file_name_args {
  char *input_file_name;     // e.g., file1.ppm
  char output_file_name[20]; // will take the form laplaciani.ppm, e.g.,
                             // laplacian1.ppm
};

/*The total_elapsed_time is the total time taken by all threads
to compute the edge detection of all input images .
*/
double total_elapsed_time = 0;

/*This is the thread function. It will compute the new values for the region of
   image specified in params (start to start+size) using convolution. For each
   pixel in the input image, the filter is conceptually placed on top ofthe
   image with its origin lying on that pixel. The  values  of  each  input image
   pixel  under  the  mask  are  multiplied  by the corresponding filter values.
    Truncate values smaller than zero to zero and larger than 255 to 255.
    The results are summed together to yield a single output value that is
   placed in the output image at the location of the pixel being processed on
   the input.

 */
void *compute_laplacian_threadfn(void *params) {

  int laplacian[FILTER_WIDTH][FILTER_HEIGHT] = {
      {-1, -1, -1}, {-1, 8, -1}, {-1, -1, -1}};

  int red, green, blue;
  struct parameter *param_lad = (struct parameter*) params;
  printf("start: %lu size: %lu\n", param_lad->start, param_lad->size);

  return NULL;
}

/* Apply the Laplacian filter to an image using threads.
 Each thread shall do an equal share of the work, i.e. work=height/number of
 threads. If the size is not even, the last thread shall take the rest of the
 work. Compute the elapsed time and store it in *elapsedTime (Read about
 gettimeofday). Return: result (filtered image)
 */
PPMPixel *apply_filters(PPMPixel *image, unsigned long w, unsigned long h,
                        double *elapsedTime) {
  printf("applying filters\n");
  // Get the start time

  // Array of pixels to output
  PPMPixel *result;
  // Initialize array of threads
  pthread_t *threads = calloc(LAPLACIAN_THREADS, sizeof(pthread_t));

  // Split the image into height/ Numthreads chunks
  // TODO: Fix this so it divides the work by rows

  int section_size = h / LAPLACIAN_THREADS;
  // create a struct parameter for each thread and pass it to pthreat_create with compute_laplacian_threadfn, each thread gets its owns sectionsize sized section
  for (int i = 0; i < LAPLACIAN_THREADS; i++) {
    struct parameter *params = malloc(sizeof(struct parameter));
    params->image = image;
    params->w = w;
    params->h = h;
    params->start = i * section_size;
    if (i == LAPLACIAN_THREADS - 1) {
      params->size = h - (i * section_size);
    } else {
      params->size = section_size;
    }
    pthread_create(&threads[i], NULL, compute_laplacian_threadfn, params);
  }

  void *res = NULL;
  for (int i = 0; i < LAPLACIAN_THREADS; i++) {
    pthread_join(threads[i], &res);
  }

  return result;
}

/*Create a new P6 file to save the filtered image in. Write the header block
 e.g. P6
      Width Height
      Max color value
 then write the image data.
 The name of the new file shall be "filename" (the second argument).
 */
void write_image(PPMPixel *image, char *filename, unsigned long int width,
                 unsigned long int height) {}

/* Open the filename image for reading, and parse it.
    Example of a ppm header:    //http://netpbm.sourceforge.net/doc/ppm.html
    P6                  -- image format
    # comment           -- comment lines begin with
    ## another comment  -- any number of comment lines
    200 300             -- image width & height
    255                 -- max color value

 Check if the image format is P6. If not, print invalid format error message.
 If there are comments in the file, skip them. You may assume that comments
 exist only in the header block. Read the image size information and store them
 in width and height. Check the rgb component, if not 255, display error
 message. Return: pointer to PPMPixel that has the pixel data of the input image
 (filename).The pixel data is stored in scanline order from left to right (up to
 bottom) in 3-byte chunks (r g b values for each pixel) encoded as binary
 numbers.
 */
PPMPixel *read_image(const char *filename, unsigned long int *width,
                     unsigned long int *height) {

  PPMPixel *img = NULL;

  // Open the image file
  FILE *fp = fopen(filename, "r");
  if (fp == NULL) {
    perror("Failed to open image file with");
    exit(EXIT_FAILURE);
  }

  // Check the format is p6
  size_t buf_size = 100;
  char *line = malloc(buf_size * sizeof(char));
  if (line == NULL) {
    perror("failed call to malloc");
    exit(EXIT_FAILURE);
  }

  fgets(line, buf_size, fp);

  // Skip comments
  while (strchr(line, '#') != NULL) {
    fgets(line, buf_size, fp);
    printf("skipping comment");
  }

  // Compare the first non comment argument to P6
  if (strcmp(line, "P6\n") != 0) {
    fprintf(stderr, "image format is not P6");
    exit(EXIT_FAILURE);
  }

  fgets(line, buf_size, fp);

  // Skip comments
  while (strchr(line, '#') != NULL) {
    fgets(line, buf_size, fp);
    // printf("skipping comment");
  }

  // Read in width and height
  char *rest = line;
  unsigned long w = atoi(strtok_r(line, " ", &rest));
  unsigned long h = atoi(strtok_r(NULL, " ", &rest));
  *width = w;
  *height = h;

  if (*width == 0 || *height == 0) {
    fprintf(stderr, "failed to parse width and height");
    exit(EXIT_FAILURE);
  }

  printf("width: %lu height: %lu\n", *width, *height);

  fgets(line, buf_size, fp);
  if (atoi(line) < 255) {
    fprintf(stderr, "max color value lower than 255");
  }

  img = calloc(sizeof(PPMPixel), w * h);
  if (img == NULL) {
    perror("failed call to malloc");
    exit(EXIT_FAILURE);
  }

  // Read the pixel data
  int bytes_read = fread(img, sizeof(PPMPixel), w * h, fp);
  if (bytes_read == 0) {
    perror("failed to read in image");
    exit(EXIT_FAILURE);
  } else {
    printf("bytes read: %d\n", bytes_read);
  }

  free(line);
  return img;
}

/* The thread function that manages an image file.
 Read an image file that is passed as an argument at runtime.
 Apply the Laplacian filter.
 Update the value of total_elapsed_time.
 Save the result image in a file called laplaciani.ppm,
     where i is the image file order in the passed arguments.
 Example: the result image of the file passed third during the input
     shall be called "laplacian3.ppm".
*/
void *manage_image_file(void *args) {
  printf("thread spawned, args: %s\n", (char*) args);

  unsigned long int w, h;
  PPMPixel *original_image = read_image(args, &w, &h);

  PPMPixel *result = apply_filters(original_image, w, h, &total_elapsed_time);
}

/*The driver of the program. Check for the correct number of arguments. If wrong
  print the message: "Usage ./a.out filename[s]" It shall accept n filenames as
  arguments, separated by whitespace, e.g., ./a.out file1.ppm file2.ppm
  file3.ppm It will create a thread for each input file to manage. It will print
  the total elapsed time in .4 precision seconds(e.g., 0.1234 s).
 */
int main(int argc, char *argv[]) {
  if (argc < 2) {
    fprintf(stderr, "Less than two arguments");
    exit(EXIT_FAILURE);
  }

  // Initialize array of threads
  pthread_t *threads = calloc(argc - 1, sizeof(pthread_t));

  // Create a thread for every image file
  for (int i = 1; i < argc; i++) {
    pthread_create(&threads[i], NULL, manage_image_file, argv[i]);
  }

  // Join the threads
  void *res = NULL;
  for (int i = 1; i < argc; i++) {
    pthread_join(threads[i], &res);
  }

  // TODO: Free the struct params
  return 0;
}
