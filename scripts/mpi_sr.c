#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <time.h>
#include <string.h>

int main(int argc, char* argv[]) {

    if(argc < 2 || argc > 4) {
        printf("Usage: %s N [mode]\n", argv[0]);
        return 1;
    }

    double start, end;
    double local_time, global_time;

    MPI_Init(&argc, &argv);

    int size, my_rank, N;
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    
    N = atoi(argv[1]);

    int exchange_mode = 0; // default: Sendrecv
    if (argc >= 3) {
        exchange_mode = atoi(argv[2]);
    }

    double * A; // Master matrix
    int * T; // Result matrix
    
    // Number of rows per process (simplified distribution)
    int base_rows = N / size;
    int extra_rows = N % size;

    // It says me how many elements each process will receive
    int *sendcounts = malloc(size * sizeof(int));
    
    // It says me the displacement (in elements) from the beginning of A_raw for each process
    int *senddispls = malloc(size * sizeof(int));

    // Master initializes matrix A
    if (my_rank == 0) {
        // Allocate memory for A and T (contiguous allocation)
        A = malloc(N * N * sizeof(double));
        T = malloc(N * N * sizeof(int));

        srand((unsigned int)time(NULL));
        
        for(int i = 0; i < N; i++){
            for(int j = 0; j < N; j++){
                A[i*N + j] = ((double)rand() / RAND_MAX) * 100.0;
                T[i*N + j] = 0;  // initialize T to zero
            }
        }

        /* Calculation of sendcounts and senddispls */
        int current_displ = 0;
        for (int p = 0; p < size; p++) {
            // distribute extra rows to the first 'extra_rows' processes
            int actual_rows = base_rows + (p < extra_rows ? 1 : 0);
            
            sendcounts[p] = actual_rows * N; 
            senddispls[p] = current_displ;
            current_displ += sendcounts[p];
        }
    }

    // Broadcast sendcounts and senddispls to all processes
    MPI_Bcast(sendcounts, size, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(senddispls, size, MPI_INT, 0, MPI_COMM_WORLD);

    // the number of rows assigned to this process
    int my_rows = sendcounts[my_rank] / N;

    // local buffers: each process gets its chunk of A and T (only actual rows, no padding)
    double *my_A = NULL;
    int *my_T = NULL;
    
    if (my_rows > 0) {
        my_A = malloc( sendcounts[my_rank] * sizeof(double));
        my_T = malloc( sendcounts[my_rank] * sizeof(int));
    }

    /* timing start **************************************/
    MPI_Barrier(MPI_COMM_WORLD);
    start = MPI_Wtime();
    /* timing start **************************************/

    /* MASTER: DATA DISTRIBUTION */
    MPI_Scatterv(
        A, 
        sendcounts, 
        senddispls, 
        MPI_DOUBLE,  // send buffers
        my_A, 
        sendcounts[my_rank], 
        MPI_DOUBLE, // recv buffer
        0, 
        MPI_COMM_WORLD
    );

    /* 1) neighbor ranks */
    int up   = (my_rank > 0)        ? my_rank - 1 : MPI_PROC_NULL;
    int down = (my_rank < size - 1) ? my_rank + 1 : MPI_PROC_NULL;
    int total_rows = my_rows + 2; // two ghost rows (above and below)

    // the last process that contains data
    int last_rank_with_data = size - 1;
    while (last_rank_with_data >= 0 && sendcounts[last_rank_with_data] == 0) {
        last_rank_with_data--;
    }

    /* 2) add ghost rows above and below */
    double *my_A_plus_ghosts = calloc(total_rows * N, sizeof(double));
    double *upper_ghost = my_A_plus_ghosts;                          // row -1
    double *local_data  = my_A_plus_ghosts + N;                 // rows [0 ... my_rows-1]
    double *lower_ghost = my_A_plus_ghosts + (my_rows + 1) * N; // row + my_rows

    /* copy local block into the central part of the buffer */
    if (my_rows > 0) {
        memcpy(local_data, my_A, my_rows * N * sizeof(double));
    }

    /* 2) Halo exchange rows */
    if (exchange_mode == 0) {
        // SSend / Recv version ********************************************************************
        if (my_rank > 0 && my_rows > 0) {
            MPI_Ssend(local_data, N, MPI_DOUBLE, up, 100, MPI_COMM_WORLD);
            MPI_Recv(upper_ghost, N, MPI_DOUBLE, up, 200, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        }

        int send_down_offset = (my_rows > 0 ? (my_rows - 1) * N : 0);
    
        if (my_rank < size - 1 && my_rows > 0) {
            if (sendcounts[down] > 0) {
                MPI_Recv(lower_ghost, N, MPI_DOUBLE, down, 100, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                MPI_Ssend(local_data + send_down_offset, N, MPI_DOUBLE, down, 200, MPI_COMM_WORLD);
            }
        }
    } else if (exchange_mode == 1) { 
        // Isend / Irecv version ******************************************************************** 
        MPI_Request requests[4];
        int req_count = 0;
        int send_down_offset = (my_rows > 0 ? (my_rows - 1) * N : 0);
    
        // receive first, then send (Irecv + Isend)
        if (my_rank > 0 && my_rows > 0) {
            MPI_Irecv(upper_ghost, N, MPI_DOUBLE, up, 200, MPI_COMM_WORLD, &requests[req_count++]);
        }
        if (my_rank < size - 1 && my_rows > 0 && sendcounts[down] > 0) {
            MPI_Irecv(lower_ghost, N, MPI_DOUBLE, down, 100, MPI_COMM_WORLD, &requests[req_count++]);
        }
    
        // send after
        if (my_rank > 0 && my_rows > 0) {
            MPI_Isend(local_data, N, MPI_DOUBLE, up, 100, MPI_COMM_WORLD, &requests[req_count++]);
        }
        if (my_rank < size - 1 && my_rows > 0 && sendcounts[down] > 0) {
            MPI_Isend(local_data + send_down_offset, N, MPI_DOUBLE, down, 200, MPI_COMM_WORLD, &requests[req_count++]);
        }
    
        // wait for all to complete
        if (req_count > 0) {
            MPI_Waitall(req_count, requests, MPI_STATUSES_IGNORE);
        }
    } else if (exchange_mode == 2) {
        // Sendrecv version ********************************************************************
        int send_down_offset = (my_rows > 0 ? (my_rows - 1) * N : 0);
        
        // Exchange from UP: send my first row, receive the row above me
        MPI_Sendrecv(local_data, N, MPI_DOUBLE, up, 100,
                    upper_ghost, N, MPI_DOUBLE, up, 200,
                    MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        
        // Exchange from DOWN: send my last row, receive the row below me
        MPI_Sendrecv(local_data + send_down_offset, N, MPI_DOUBLE, down, 200,
                    lower_ghost, N, MPI_DOUBLE, down, 100,
                    MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    } 
    
    /* 3. Parallel processing (each process on its local domain) */
    for (int i = 0; i < my_rows; i++) { // handles only valid rows
        for (int j = 0; j < N; j++) {

            // initialize accumulators
            double sum = 0; // sum of the neighborhood values
            int count = 0;    // number of elements, for the mean

            /* 3.1 neighborhood  bounds */
            // vertical 
            int zmin = (i == 0 && my_rank == 0) ? 0 : -1; // top boundary
            int zmax = (i == my_rows - 1 && my_rank == last_rank_with_data) ? 0 : 1; // bottom boundary

            // horizontal
            int wmin = (j > 0) ? -1 : 0; // left boundary
            int wmax = (j < N - 1) ? 1 : 0; // right boundary

            /* 3.2 scan the 3x3 neighborhood */
            for (int dz = zmin; dz <= zmax; dz++) { // vertical offset
                int rowIndex = i + 1 + dz; // +1 to skip upper ghost

                for (int dw = wmin; dw <= wmax; dw++) { // horizontal offset
                    int colIndex = j + dw;

                    sum += my_A_plus_ghosts[rowIndex * N + colIndex];
                    count++;
                }
            }

            /* 3.5 thresholding operation */
            my_T[i * N + j] = (local_data[i * N + j] * count > sum) ? 1 : 0;
        }
    }

    free(my_A_plus_ghosts);

    /* MASTER: DATA GATHERING */
    MPI_Gatherv(
        my_T, 
        my_rows * N, 
        MPI_INT,      // send buffer
        T, 
        sendcounts, 
        senddispls, 
        MPI_INT, // recv buffers
        0, 
        MPI_COMM_WORLD
    );

    end = MPI_Wtime();
    local_time = end - start;

    MPI_Reduce(
        &local_time, 
        &global_time, 
        1, 
        MPI_DOUBLE, 
        MPI_MAX, 
        0, 
        MPI_COMM_WORLD
    );

    if (my_rank == 0) {
        free(A);
        free(T);
    }

    if (my_rows > 0) {
        free(my_A);
        free(my_T);
    }

    free(sendcounts);
    free(senddispls);
    
    MPI_Finalize();

    if (my_rank == 0) {
        printf("%lf\n", global_time);
    }

    return EXIT_SUCCESS;
}