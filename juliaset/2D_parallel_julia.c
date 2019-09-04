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
    int row_size = n / array_dim;
    int column_size = (2 * n) / array_dim;
    int rgb_pixels = 3;
    float tint_bias = pow(rank, 2); 
    int pixels_cell_size = row_size * column_size * rgb_pixels;
    unsigned char* pixels_cell = malloc(sizeof(unsigned char) * pixels_cell_size);    

    MPI_Comm new_comm;
    int coord[2];

    get_process_coord(new_comm, coord, array_dim, rank);

    int initial_heigth_pos = coord[0] * row_size;
    int last_heigth_pos = (coord[0] + 1) * row_size;
    int initial_width_pos = coord[1] * column_size;
    int last_width_pos = (coord[1] + 1) * column_size;

    printf("[Process rank %d]: my 2-D rank is (%d, %d), my tile is [%d:%d, %d:%d]\n", rank, coord[0], coord[1], initial_heigth_pos, last_heigth_pos, initial_width_pos, last_width_pos);
    
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
        fp = fopen("julia2d.bpm", "w");
        // Write BPM Header
        int res = write_bmp_header(fp, width, heigth);
        int tile_size = column_size * rgb_pixels;
        int section_size = row_size;
        // Write first section and close the file
        fwrite(&pixels_cell, sizeof(char), tile_size, fp);
        fclose(fp);
        MPI_Send(message, sizeof(message), MPI_BYTE, 1, 1, MPI_COMM_WORLD);
        for (int j = 1; j < section_size; j++) {
            MPI_Recv(buffer, sizeof(message), MPI_BYTE, 2, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            fp = fopen("julia2d.bpm", "a");
            fwrite(&pixels_cell[rgb_pixels * (j * column_size)], sizeof(char), tile_size, fp);
            fclose(fp);
            MPI_Send(message, sizeof(message), MPI_BYTE, 1, 1, MPI_COMM_WORLD);
        }
        MPI_Send(message, sizeof(message), MPI_BYTE, 3, 1, MPI_COMM_WORLD);
    } else if (rank == 1) {
        int tile_size = column_size * rgb_pixels;
        int section_size = row_size;
        FILE *fp;
        for (int j = 0; j < section_size; j++) {
            MPI_Recv(buffer, sizeof(message), MPI_BYTE, 0, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            fp = fopen("julia2d.bpm", "a");
            fwrite(&pixels_cell[rgb_pixels * (j * column_size)], sizeof(char), tile_size, fp);
            fclose(fp);
            MPI_Send(message, sizeof(message), MPI_BYTE, 2, 1, MPI_COMM_WORLD);
        }
    } else if (rank == 2) {
        int tile_size = column_size * rgb_pixels;
        int section_size = row_size;          
        FILE *fp;
        for (int j = 0; j < section_size; j++) {
            MPI_Recv(buffer, sizeof(message), MPI_BYTE, 1, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            fp = fopen("julia2d.bpm", "a");
            fwrite(&pixels_cell[rgb_pixels * (j * column_size)], sizeof(char), tile_size, fp);
            fclose(fp);
            MPI_Send(message, sizeof(message), MPI_BYTE, 0, 1, MPI_COMM_WORLD);
        }
    } else if (rank == 3) {
        int tile_size = column_size * rgb_pixels;
        int section_size = row_size;
        FILE *fp;
        MPI_Recv(buffer, sizeof(message), MPI_BYTE, 0, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        fp = fopen("julia2d.bpm", "a");
        fwrite(&pixels_cell, sizeof(char), tile_size, fp);
        fclose(fp);
        MPI_Send(message, sizeof(message), MPI_BYTE, 4, 1, MPI_COMM_WORLD);
        for (int j = 1; j < section_size; j++) {
            MPI_Recv(buffer, sizeof(message), MPI_BYTE, 5, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            fp = fopen("julia2d.bpm", "a");
            fwrite(&pixels_cell[rgb_pixels * (j * column_size)], sizeof(char), tile_size, fp);
            fclose(fp);
            MPI_Send(message, sizeof(message), MPI_BYTE, 4, 1, MPI_COMM_WORLD);
        }
        MPI_Send(message, sizeof(message), MPI_BYTE, 6, 1, MPI_COMM_WORLD);
    } else if (rank == 4) {
        int tile_size = column_size * rgb_pixels;
        int section_size = row_size;
        FILE *fp;
        for (int j = 0; j < section_size; j++) {
            MPI_Recv(buffer, sizeof(message), MPI_BYTE, 3, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            fp = fopen("julia2d.bpm", "a");
            fwrite(&pixels_cell[rgb_pixels * (j * column_size)], sizeof(char), tile_size, fp);
            fclose(fp);
            MPI_Send(message, sizeof(message), MPI_BYTE, 5, 1, MPI_COMM_WORLD);
        }
    } else if (rank == 5) {
        int tile_size = column_size * rgb_pixels;
        int section_size = row_size;
        FILE *fp;
        for (int j = 0; j < section_size; j++) {
            MPI_Recv(buffer, sizeof(message), MPI_BYTE, 4, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            fp = fopen("julia2d.bpm", "a");
            fwrite(&pixels_cell[rgb_pixels * (j * column_size)], sizeof(char), tile_size, fp);
            fclose(fp);
            MPI_Send(message, sizeof(message), MPI_BYTE, 3, 1, MPI_COMM_WORLD);
        }
    } else if (rank == 6) {
        int tile_size = column_size * rgb_pixels;
        int section_size = row_size;
        FILE *fp;
        MPI_Recv(buffer, sizeof(message), MPI_BYTE, 3, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        fp = fopen("julia2d.bpm", "a");
        fwrite(&pixels_cell, sizeof(char), tile_size, fp);
        fclose(fp);
        MPI_Send(message, sizeof(message), MPI_BYTE, 7, 1, MPI_COMM_WORLD);
        for (int j = 1; j < section_size; j++) {
            MPI_Recv(buffer, sizeof(message), MPI_BYTE, 8, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            fp = fopen("julia2d.bpm", "a");
            fwrite(&pixels_cell[rgb_pixels * (j * column_size)], sizeof(char), tile_size, fp);
            fclose(fp);
            MPI_Send(message, sizeof(message), MPI_BYTE, 7, 1, MPI_COMM_WORLD);
        }
    } else if (rank == 7) {
        int tile_size = column_size * rgb_pixels;
        int section_size = row_size;
        FILE *fp;
        for (int j = 0; j < section_size; j++) {
            MPI_Recv(buffer, sizeof(message), MPI_BYTE, 6, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            fp = fopen("julia2d.bpm", "a");
            fwrite(&pixels_cell[rgb_pixels * (j * column_size)], sizeof(char), tile_size, fp);
            fclose(fp);
            MPI_Send(message, sizeof(message), MPI_BYTE, 8, 1, MPI_COMM_WORLD);
        } 
    } else if (rank == 8) {
        int tile_size = column_size * rgb_pixels;
        int section_size = row_size;
        FILE *fp;
        for (int j = 0; j < section_size; j++) {
            MPI_Recv(buffer, sizeof(message), MPI_BYTE, 7, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            fp = fopen("julia2d.bpm", "a");
            fwrite(&pixels_cell[rgb_pixels * (j * column_size)], sizeof(char), tile_size, fp);
            fclose(fp);
            MPI_Send(message, sizeof(message), MPI_BYTE, 6, 1, MPI_COMM_WORLD);
        }
    }
    
    MPI_Finalize();
}