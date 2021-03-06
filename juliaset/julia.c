#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <mpi.h>

/*
 * compute_julia_pixel(): compute RBG values of a pixel in a
 *                        particular Julia set image.
 *
 *  In:
 *      (x,y):            pixel coordinates
 *      (width, height):  image dimensions
 *      tint_bias:        a float to "tweak" the tint (1.0 is "no tint")
 *  Out:
 *      rgb: an already-allocated 3-byte array into which R, G, and B
 *           values are written.
 *
 *  Return:
 *      0 in success, -1 on failure
 *
 */

int compute_julia_pixel(int x, int y, int width, int height, float tint_bias, unsigned char *rgb) {

  // Check coordinates
  if ((x < 0) || (x >= width) || (y < 0) || (y >= height)) {
    fprintf(stderr,"Invalid (%d,%d) pixel coordinates in a %d x %d image\n", x, y, width, height);
    return -1;
  }

  // "Zoom in" to a pleasing view of the Julia set
  float X_MIN = -1.6, X_MAX = 1.6, Y_MIN = -0.9, Y_MAX = +0.9;
  float float_y = (Y_MAX - Y_MIN) * (float)y / height + Y_MIN ;
  float float_x = (X_MAX - X_MIN) * (float)x / width  + X_MIN ;

  // Point that defines the Julia set
  float julia_real = -.79;
  float julia_img = .15;

  // Maximum number of iteration
  int max_iter = 300;

  // Compute the complex series convergence
  float real=float_y, img=float_x;
  int num_iter = max_iter;
  while (( img * img + real * real < 2 * 2 ) && ( num_iter > 0 )) {
    float xtemp = img * img - real * real + julia_real;
    real = 2 * img * real + julia_img;
    img = xtemp;
    num_iter--;
  }

  // Paint pixel based on how many iterations were used, using some funky colors
  float color_bias = (float) num_iter / max_iter;
  rgb[0] = (num_iter == 0 ? 200 : - 500.0 * pow(tint_bias, 1.2) *  pow(color_bias, 1.6));
  rgb[1] = (num_iter == 0 ? 100 : -255.0 *  pow(color_bias, 0.3));
  rgb[2] = (num_iter == 0 ? 100 : 255 - 255.0 * pow(tint_bias, 1.2) * pow(color_bias, 3.0));
  return 0;
}
/* 
 * debug_pixels():
 *
 *  Auxiliar function to print all pixels in a image.
 * 
 */
void debug_pixels(unsigned char *pixels, size_t length) {
    for (int index = 0; index < length; index ++) {
        printf("%d ", pixels[index]);
    }
    printf("\n");
}

/*
 * is_last_process():
 *
 * Checks if the process is the last one.
 *
 *   In:
 *      rank: Number of current process
 *      size: Number total of process
 *
 *   Return:
 *      Boolean value 1 (true) ou 0 (false).
*/

int is_last_process(int rank, int size) {
  return (rank+1) % size == 0;
}

/*
 * is_not_last_process_in_cartesian_plan():
 *
 * Checks if the process is the last one, in a cartesian plan distribution.
 *
 *   In:
 *      rank: Number of current process
 *      size: Number total of process
 *      matrix_dim: Number N of a matrix NxN.
 *
 *   Return:
 *      Boolean value 1 (true) ou 0 (false).
*/
int is_not_last_process_in_cartesian_plan(int rank, int matrix_dim, int size) {
  return (rank + matrix_dim) < size;
}

/* write_bmp_header():
 *
 *   In:
 *      f: A file open for writing ('w') 
 *      (width, height): image dimensions
 *   
 *   Return:
 *      0 on success, -1 on failure
 *
 */

int write_bmp_header(FILE *f, int width, int height) {

  unsigned int row_size_in_bytes = width * 3 * height;

  // Define all fields in the bmp header
  char id[2] = "BM";
  unsigned int filesize = 54 + (int)(row_size_in_bytes * height * sizeof(char));
  short reserved[2] = {0,0};
  unsigned int offset = 54;

  unsigned int size = 40;
  unsigned short planes = 1;
  unsigned short bits = 24;
  unsigned int compression = 0;
  unsigned int image_size = width * height * 3 * sizeof(char);
  int x_res = 0;
  int y_res = 0;
  unsigned int ncolors = 0;
  unsigned int importantcolors = 0;

  // Write the bytes to the file, keeping track of the
  // number of written "objects"
  size_t ret = 0;
  ret += fwrite(id, sizeof(char), 2, f);
  ret += fwrite(&filesize, sizeof(int), 1, f);
  ret += fwrite(reserved, sizeof(short), 2, f);
  ret += fwrite(&offset, sizeof(int), 1, f);
  ret += fwrite(&size, sizeof(int), 1, f);
  ret += fwrite(&width, sizeof(int), 1, f);
  ret += fwrite(&height, sizeof(int), 1, f);
  ret += fwrite(&planes, sizeof(short), 1, f);
  ret += fwrite(&bits, sizeof(short), 1, f);
  ret += fwrite(&compression, sizeof(int), 1, f);
  ret += fwrite(&image_size, sizeof(int), 1, f);
  ret += fwrite(&x_res, sizeof(int), 1, f);
  ret += fwrite(&y_res, sizeof(int), 1, f);
  ret += fwrite(&ncolors, sizeof(int), 1, f);
  ret += fwrite(&importantcolors, sizeof(int), 1, f);

  // Success means that we wrote 17 "objects" successfully
  return (ret != 17);
}

/*
 * add_pixels():
 *
 * Sum all pixels values between two pixels arrays and return the result.
 *
 *   In:
 *      pixels_array_1: First pixel array.
 *      pixels_array_2: Second pixel array.
 *
 *   Return:
 *      Array with all positions summed.
*/

unsigned char* add_pixels(unsigned char *pixels_array_1, unsigned char *pixels_array_2, size_t pixels_size) {
  unsigned char *result = malloc(sizeof(char) * pixels_size);
  
  for (int index = 0; index < pixels_size; index ++) {
    result[index] = pixels_array_1[index] + pixels_array_2[index];
  }

  return result;
}

/*
 * get_pixels_file():
 *
 * Return all pixels saved in a file.
 *
 *   In:
 *      pixels_size: Number of pixels saved in the file.
 *      fp: Pointer to the file.
 *
 *   Return:
 *      Array with pixels.
 */

void get_pixels_file(unsigned char* buffer, size_t pixels_size, char* filename) {
  FILE *fp;
  fp = fopen(filename, "r");
  size_t result;
  // Shift BPM Header information
  fseek(fp, 54, SEEK_SET);
  // Read all pixels that already had been saved
  result = fread(buffer, 1, pixels_size, fp);
  fclose(fp);
}

/* 
 * get_process_coord():
 * 
 * Return the specific coordenates of a process in a cartesian plan.
 * 
 *    In: 
 *       comm: MPI Communicator.
 *       coord[2]: Array where coordenates will be saved.
 *       array_dim: Matrix NxN dimension.
 *       rank: Rank of a specific process.
 * 
 *    Return:
 *      Coordenate (x,y) will be returned on coord array.
 *      coord[0] = x
 *      coord[1] = y 
 */
void get_process_coord(MPI_Comm comm, int coord[2], int array_dim, int rank) {
  int ndims, reorder, ierr;
  int dim_size[2], periods[2];

  ndims = 2;
  dim_size[0] = array_dim;
  dim_size[1] = array_dim;
  periods[0] = 1;
  periods[1] = 0;
  reorder = 1;

  ierr =  MPI_Cart_create(MPI_COMM_WORLD, ndims, dim_size, periods, reorder, &comm);
  MPI_Cart_coords(comm, rank, array_dim, coord);
}