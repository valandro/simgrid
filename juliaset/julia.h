#ifndef JULIA_H_
#define JULIA_H_

int compute_julia_pixel(int x, int y, int width, int height, float tint_bias, unsigned char *rgb);
int write_bmp_header(FILE *f, int width, int height);
void debug_pixels(unsigned char *pixels, size_t length);

#endif