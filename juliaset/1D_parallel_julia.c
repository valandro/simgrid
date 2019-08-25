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
    int pixels_size = heigth * width * 3;
    float tint_bias = 1.0;
    unsigned char *pixels_row = malloc(sizeof(char) * pixels_size);

    int initial_heigth_pos = (((rank) % size) * rows);
    int last_heigth_pos = (((rank + 1) == size) ? (size * rows) : (((rank + 1) % size) * rows));

    for (int y = initial_heigth_pos; y < last_heigth_pos; y++) {
        for (int x = 0; x < width; x++) {
            int result = compute_julia_pixel(x, y, width, heigth, tint_bias, &pixels_row[x * n + y]);
        }
    }

    char message[10] = "Go ahead!";
    char *buffer = malloc(sizeof(char) * 11);

    if (rank == 0) {
        FILE *fp;
        fp = fopen("julia1d.bpm", "w");

        int res = write_bmp_header(fp, width, heigth);
        fwrite(pixels_row, sizeof(char), pixels_size, fp);

        fclose(fp);
        MPI_Send(message, sizeof(message), MPI_BYTE, 1, 1, MPI_COMM_WORLD);
    } else {
        MPI_Recv(buffer, sizeof(buffer), MPI_BYTE, rank - 1, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        FILE *fp;
        // Open the file for reading the current pixels saved
        fp = fopen("julia1d.bpm", "r");
        unsigned char *buffer = malloc(sizeof(char) * pixels_size);
        size_t result;
        // Shift BPM Header information
        fseek(fp, 54, SEEK_SET);
        // Read all pixels that already had been saved
        result = fread(buffer, 1, pixels_size, fp);
        // Merge the file pixels with the pixels_row that was calculated by this process     
        unsigned char* pixels_added = add_pixels(buffer, pixels_row, pixels_size);
        fclose(fp);
        // Open file for saving the new pixels merged
        fp = fopen("julia1d.bpm", "w");
        // Rewrite BPM Header
        int res = write_bmp_header(fp, width, heigth);
        fwrite(pixels_added, sizeof(char), pixels_size, fp);

        // Free pointers
        free(pixels_added);
        free(buffer);
        fclose(fp);
        // The last process doesnt send an MPI Message
        if (!is_last_process(rank, size)) {
            MPI_Send(message, sizeof(message), MPI_BYTE, (rank + 1) % size, 1, MPI_COMM_WORLD);
        }
    }

    MPI_Finalize();
}