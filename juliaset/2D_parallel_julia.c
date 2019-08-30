#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <math.h>
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
    int array_dim = sqrt(size);
    int rows = n / array_dim;
    int columns = (2 * n) / array_dim;
    int rgb_pixels = 3;
    int pixels_cell_size = rows * columns * rgb_pixels;
    unsigned char* pixels_cell = malloc(sizeof(unsigned char) * pixels_cell_size);    

    MPI_Comm new_comm;
    int coord[2];

    get_process_coord(new_comm, coord, array_dim, rank);
    printf("[Process rank %d]: (%d, %d)\n", rank, coord[0], coord[1]);
    MPI_Finalize();
}