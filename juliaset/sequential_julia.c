#include <stdio.h>
#include <stdlib.h>
#include "julia.h"

int main(int argc, char *argv[]) {
    /* 
     *   Size of an Julia's set image => argv[1]
     *
     *   HEIGTH => n pixels
     *   WIDTH => 2n pixels
     */
    int n = atoi(argv[1]);

    if (n < 0) {
        printf("N should be a positive number!\n");
        return 0;
    }

    int width = 2 * n;
    int heigth = n;
    int rgb_pixels = 3;
    int pixels_size = width * heigth * rgb_pixels;
    unsigned char *pixels = malloc(sizeof(unsigned char) * pixels_size);
    float tint_bias = 1.0;

    // Compute pixels
    for (int x = 0; x < width; x++) {
        for (int y = 0; y < heigth; y++) {
            int result = compute_julia_pixel(x, y, width, heigth, tint_bias, &pixels[rgb_pixels * ((y * width) + x)]);
        }
    }

    // Write file with the pixels value
    FILE *fp;
    fp = fopen("julia.bpm", "w");
    int res = write_bmp_header(fp, width, heigth);
    fwrite(pixels, sizeof(char), pixels_size, fp);
    fclose(fp);

    return 0;
}