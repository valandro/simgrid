#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
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

    int size, rank;
    char hostname[1024];
    int hostname_len;

    MPI_Init(&argc, &argv);

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Get_processor_name(hostname, &hostname_len);

    int width = 2 * n;
    int heigth = n;
    int rows = n / size;
    int rgb_pixels = 3;
    int pixels_size = width * rows * rgb_pixels;
    float tint_bias = 1.0;
    unsigned char *pixels_row = malloc(sizeof(char) * pixels_size);

    int initial_heigth_pos = (((rank) % size) * rows);
    int last_heigth_pos = (((rank + 1) == size) ? (size * rows) : (((rank + 1) % size) * rows));

    for (int x = 0; x < width; x++) {
        for (int y = initial_heigth_pos, row = 0; y < last_heigth_pos && row < rows; y++, row++) {
            int result = compute_julia_pixel(x, y, width, heigth, tint_bias, &pixels_row[rgb_pixels * ((row * width) + x)]);
        }
    }

    char message[10] = "Go ahead!";
    char *buffer = malloc(sizeof(char) * 11);

    if (rank == 0) {
        FILE *fp;
        fp = fopen("julia1d.bpm", "w");
        // Write BPM Header
        int res = write_bmp_header(fp, width, heigth);
        // Write pixel values
        fwrite(pixels_row, sizeof(char), pixels_size, fp);
        fclose(fp);
        // Send a message to the next process
        MPI_Send(message, sizeof(message), MPI_BYTE, 1, 1, MPI_COMM_WORLD);
    } else {
        MPI_Recv(buffer, sizeof(buffer), MPI_BYTE, rank - 1, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        // Open and save all pixels at EOF
        FILE *fp;
        fp = fopen("julia1d.bpm", "a");
        fwrite(pixels_row, sizeof(char), pixels_size, fp);

        // Free pointers
        fclose(fp);
        // The last process doesnt send an MPI Message
        if (!is_last_process(rank, size)) {
            MPI_Send(message, sizeof(message), MPI_BYTE, (rank + 1) % size, 1, MPI_COMM_WORLD);
        }
    }

    MPI_Finalize();
}