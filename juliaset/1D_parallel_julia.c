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
    struct timeval start, end;
    char hostname[1024];
    int hostname_len;

    MPI_Init(&argc, &argv);

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Get_processor_name(hostname, &hostname_len);

    int width = 2 * n;
    int heigth = n;
    int pixels_size = heigth * width * 3;
    int rows = n / size;

    printf("Process %d out of %d: I should compute pixels rows %d to %d, for a total of %d rows\n", rank, size, (((rank) % size) * rows) , ((rank + 1) == size) ? size * rows : (((rank + 1) % size) * rows), rows);

    MPI_Finalize();
}