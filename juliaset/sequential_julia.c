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
    int pixels_size = heigth * width * 3;
    unsigned char *pixels = malloc(sizeof(char) * pixels_size);
    float tint_bias = 1.0;

    // Compute pixels
    for (int y = 0; y < heigth; y++) {
        for (int x = 0; x < width; x++) {
            int result = compute_julia_pixel(x, y, width, heigth, tint_bias, &pixels[x * n + y]);
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