#include <stdio.h>
#include <stdlib.h>
#include "julia.h"

int main(int argc, char *argv[]) {
    /* 
     *   Size of an Julia's set image 
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
    int pixels_size = heigth * width * 3;
    unsigned char pixels[pixels_size];
    float tint_bias = 1.0;

    for (int x = 1; x < width; x++) {
        for (int y = 1; y < heigth; y++) {
            int result = compute_julia_pixel(x, y, width, heigth, tint_bias, pixels);
            if (result == -1) {
                printf("Something went wrong!\n");
            }
        }
    }
    // debug_pixels(pixels, pixels_size);
    return 0;
}