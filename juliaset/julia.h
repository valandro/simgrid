#ifndef JULIA_H_
#define JULIA_H_

int compute_julia_pixel(int x, int y, int width, int height, float tint_bias, unsigned char *rgb);
int write_bmp_header(FILE *f, int width, int height);
void debug_pixels(unsigned char *pixels, size_t length);
int is_last_process(int rank, int size);
unsigned char* add_pixels(unsigned char *pixels_array_1, unsigned char *pixels_array_2, size_t pixels_size);
void get_pixels_file(unsigned char* buffer, size_t pixels_size, char* filename);
void get_process_coord(MPI_Comm comm, int *coord, int array_dim, int rank);
#endif