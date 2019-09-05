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
    int matrix_dim = sqrt(size);
    int row_size = n / matrix_dim;
    int column_size = (2 * n) / matrix_dim;
    int rgb_pixels = 3;
    float tint_bias = pow(rank, 2); 
    int pixels_cell_size = row_size * column_size * rgb_pixels;
    unsigned char* pixels_cell = malloc(sizeof(unsigned char) * pixels_cell_size);    

    MPI_Comm new_comm;
    int coord[2];

    get_process_coord(new_comm, coord, matrix_dim, rank);

    // Set coordenates
    int initial_heigth_pos = coord[0] * row_size;
    int last_heigth_pos = (coord[0] + 1) * row_size;
    int initial_width_pos = coord[1] * column_size;
    int last_width_pos = (coord[1] + 1) * column_size;

    // Compute pixels from the respective tile    
    for (int x = initial_width_pos, column = 0; x < last_width_pos && column < column_size; x++, column++) {
        for (int y = initial_heigth_pos, row = 0; y < last_heigth_pos && row < row_size; y++, row++) {
            int result = compute_julia_pixel(x, y, width, heigth, tint_bias, &pixels_cell[rgb_pixels * ((row * column_size) + column)]);
        }
    }

    char message[10] = "Go ahead!";
    char *buffer = malloc(sizeof(char) * 11);

    if (rank == 0) {
        // First process should write the BMP Header
        FILE *fp;
        fp = fopen("julia2d.bmp", "w");
        // Write BPM Header
        int res = write_bmp_header(fp, width, heigth);
        int tile_size = column_size * rgb_pixels;
        int section_size = row_size;
        // Write first section and close the file
        fwrite(&pixels_cell, sizeof(char), tile_size, fp);
        fclose(fp);
        // Send message to process 1
        MPI_Send(message, sizeof(message), MPI_BYTE, 1, 1, MPI_COMM_WORLD);
        for (int j = 1; j < section_size; j++) {
            // Receive a message from the last process at line, then write the next section
            MPI_Recv(buffer, sizeof(message), MPI_BYTE, matrix_dim - 1, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            fp = fopen("julia2d.bmp", "a");
            fwrite(&pixels_cell[rgb_pixels * (j * column_size)], sizeof(char), tile_size, fp);
            fclose(fp);
            // Send message to process 1
            MPI_Send(message, sizeof(message), MPI_BYTE, 1, 1, MPI_COMM_WORLD);
        }
        // Send a message to the process that has a rank equals to matrix dimension (NxN)
        MPI_Send(message, sizeof(message), MPI_BYTE, matrix_dim, 1, MPI_COMM_WORLD);
    } else if (rank % matrix_dim == 0) {
        int tile_size = column_size * rgb_pixels;
        int section_size = row_size;
        FILE *fp;
        // The first process of each row should write the first section and they always receive the message
        // from the last process of the previous row
        MPI_Recv(buffer, sizeof(message), MPI_BYTE, rank - matrix_dim, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        fp = fopen("julia2d.bmp", "a");
        fwrite(&pixels_cell, sizeof(char), tile_size, fp);
        fclose(fp);
        // Send a message to next process
        MPI_Send(message, sizeof(message), MPI_BYTE, rank + 1, 1, MPI_COMM_WORLD);
        for (int j = 1; j < section_size; j++) {
            // Receive a message from the last process of the current row
            MPI_Recv(buffer, sizeof(message), MPI_BYTE, rank + matrix_dim - 1, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            fp = fopen("julia2d.bmp", "a");
            fwrite(&pixels_cell[rgb_pixels * (j * column_size)], sizeof(char), tile_size, fp);
            fclose(fp);
            // Send a message to next process
            MPI_Send(message, sizeof(message), MPI_BYTE, rank + 1, 1, MPI_COMM_WORLD);
        }
        if (is_not_last_process_in_cartesian_plan(rank, matrix_dim, size)) {
            MPI_Send(message, sizeof(message), MPI_BYTE, rank + matrix_dim, 1, MPI_COMM_WORLD);
        }
    } else if ((rank + 1) % matrix_dim == 0) {
        int tile_size = column_size * rgb_pixels;
        int section_size = row_size;
        FILE *fp;
        for (int j = 0; j < section_size; j++) {
            // Receive a message from the previous process
            MPI_Recv(buffer, sizeof(message), MPI_BYTE, rank - 1, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            fp = fopen("julia2d.bmp", "a");
            fwrite(&pixels_cell[rgb_pixels * (j * column_size)], sizeof(char), tile_size, fp);
            fclose(fp);
            // The last process of each row should send a message to the first one
            MPI_Send(message, sizeof(message), MPI_BYTE, (rank + 1) - matrix_dim, 1, MPI_COMM_WORLD);
        }
    } else {
        int tile_size = column_size * rgb_pixels;
        int section_size = row_size;
        FILE *fp;
        for (int j = 0; j < section_size; j++) {
            // If isn't a boundire process in the cartesion plan distribution, receive a message from the previous process
            MPI_Recv(buffer, sizeof(message), MPI_BYTE, rank - 1, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            fp = fopen("julia2d.bmp", "a");
            fwrite(&pixels_cell[rgb_pixels * (j * column_size)], sizeof(char), tile_size, fp);
            fclose(fp);
            // And send a message to the next process
            MPI_Send(message, sizeof(message), MPI_BYTE, rank + 1, 1, MPI_COMM_WORLD);
        }
    }
    
    MPI_Finalize();
}