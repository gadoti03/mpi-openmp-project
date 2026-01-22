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
    
    // Number of rows per process
    int base_rows = N / size;
    int extra_rows = N % size;

    // How many elements each process will receive
    int *sendcounts = malloc(size * sizeof(int));
    
    // The displacement from the beginning of A for each process
    int *senddispls = malloc(size * sizeof(int));

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

    // Number of rows assigned to this process
    int my_rows = sendcounts[my_rank] / N;

    // Local buffers
    double *my_A = NULL;
    int *my_T = NULL;
    
    if (my_rows > 0) {
        my_A = malloc( sendcounts[my_rank] * sizeof(double));
        my_T = malloc( sendcounts[my_rank] * sizeof(int));
    }

    MPI_Barrier(MPI_COMM_WORLD);
    start = MPI_Wtime();

    MPI_Scatterv(
        A, 
        sendcounts, 
        senddispls, 
        MPI_DOUBLE,
        my_A, 
        sendcounts[my_rank], 
        MPI_DOUBLE,
        0, 
        MPI_COMM_WORLD
    );

    // Neighbors ranks
    int up   = (my_rank > 0)        ? my_rank - 1 : MPI_PROC_NULL;
    int down = (my_rank < size - 1) ? my_rank + 1 : MPI_PROC_NULL;
    int total_rows = my_rows + 2;


    // Add halo rows above and below
    double *my_A_plus_halos = calloc(total_rows * N, sizeof(double));
    double *upper_halo = my_A_plus_halos; // row -1
    double *local_data  = my_A_plus_halos + N; // rows [0 ... my_rows-1]
    double *lower_halo = my_A_plus_halos + (my_rows + 1) * N; // row + my_rows

    // Copy local block into the central part of the buffer
    if (my_rows > 0) {
        memcpy(local_data, my_A, my_rows * N * sizeof(double));
    }

    // Halo exchange rows
    if (exchange_mode == 0) { // SSend / Recv version 
        if (my_rank > 0 && my_rows > 0) {
            MPI_Ssend(local_data, N, MPI_DOUBLE, up, 1, MPI_COMM_WORLD);
            MPI_Recv(upper_halo, N, MPI_DOUBLE, up, 2, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        }

        int send_down_offset = (my_rows > 0 ? (my_rows - 1) * N : 0);
    
        if (my_rank < size - 1 && my_rows > 0) {
            if (sendcounts[down] > 0) {
                MPI_Recv(lower_halo, N, MPI_DOUBLE, down, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                MPI_Ssend(local_data + send_down_offset, N, MPI_DOUBLE, down, 2, MPI_COMM_WORLD);
            }
        }
    } else if (exchange_mode == 1) { // Isend / Irecv version
        MPI_Request requests[4]; // Array of requests (=4 at max)
        int req_count = 0; // Number of requests (=4 at max)
        int send_down_offset = (my_rows > 0 ? (my_rows - 1) * N : 0);
    
        // Receive first
        if (my_rank > 0 && my_rows > 0) {
            MPI_Irecv(upper_halo, N, MPI_DOUBLE, up, 2, MPI_COMM_WORLD, &requests[req_count++]);
        }
        if (my_rank < size - 1 && my_rows > 0 && sendcounts[down] > 0) {
            MPI_Irecv(lower_halo, N, MPI_DOUBLE, down, 1, MPI_COMM_WORLD, &requests[req_count++]);
        }

        // Send after
        if (my_rank > 0 && my_rows > 0) {
            MPI_Isend(local_data, N, MPI_DOUBLE, up, 1, MPI_COMM_WORLD, &requests[req_count++]);
        }
        if (my_rank < size - 1 && my_rows > 0 && sendcounts[down] > 0) {
            MPI_Isend(local_data + send_down_offset, N, MPI_DOUBLE, down, 2, MPI_COMM_WORLD, &requests[req_count++]);
        }

        // Wait for all to complete
        if (req_count > 0) {
            MPI_Waitall(req_count, requests, MPI_STATUSES_IGNORE);
        }
    } else if (exchange_mode == 2) { // Sendrecv version
        int send_down_offset = (my_rows > 0 ? (my_rows - 1) * N : 0);
        
        // UP: send my first row, receive the row above me
        MPI_Sendrecv(local_data, N, MPI_DOUBLE, up, 1,
                    upper_halo, N, MPI_DOUBLE, up, 2,
                    MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        
        // DOWN: send my last row, receive the row below me
        MPI_Sendrecv(local_data + send_down_offset, N, MPI_DOUBLE, down, 2,
                    lower_halo, N, MPI_DOUBLE, down, 1,
                    MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    } 
    
    // Computer local sum
    for (int i = 0; i < my_rows; i++) {
        for (int j = 0; j < N; j++){ // for each column

            // initialize accumulators
            double sum = 0; // sum of the neighborhood values
            int count = 0;  // number of elements, for the mean

            // Loop over the 3x3 neighborhood
            for (int di = -1; di <= 1; di++) {
                int ni = i + di + 1; // +1 to skip the upper halo

                // check MPI halo boundaries
                if ((i == 0 && my_rank == 0 && di < 0) || 
                    (i == my_rows - 1 && my_rank == size - 1 && di > 0)) {
                    continue;
                }

                for (int dj = -1; dj <= 1; dj++) {
                    int nj = j + dj;

                    // horizontal boundaries
                    if (nj < 0 || nj >= N) continue;

                    sum += my_A_plus_halos[ni * N + nj];
                    count++;
                }
            }


            my_T[i * N + j] = (local_data[i * N + j] * count > sum) ? 1 : 0; // ottimizzazione
        }
    }    


    free(my_A_plus_halos);

    MPI_Gatherv(
        my_T, 
        my_rows * N, 
        MPI_INT,
        T, 
        sendcounts, 
        senddispls, 
        MPI_INT,
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