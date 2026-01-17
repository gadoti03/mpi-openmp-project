#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <time.h>

int main(int argc, char* argv[]) {
    if(argc != 2) {
        printf("Usage: %s N\n", argv[0]);
        return 1;
    }

    double start, end;
    double local_time, global_time;

    MPI_Init(&argc, &argv);

    int size, my_rank, N;
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);

    N = atoi(argv[1]);

    double * A; // Master matrix
    int * T; // Result matrix

    int base_rows = N / size; // rows per process
    int extra = N % size; // extra rows

    /////////////////////////////////////////////
    // Compute sendcounts and displs for Scatterv
    /////////////////////////////////////////////

    int *sendcounts;
    int *displs_scatterv;

    if (my_rank == 0) {
        sendcounts = malloc(size * sizeof(int));
        displs_scatterv = malloc(size * sizeof(int));

        int base_offset = 0; // Actual offset in elements

        for(int i=0; i<size; i++){
            // Assign an extra row to the first 'extra' processes
            int rows_i = base_rows + (i < extra ? 1 : 0);
            
            // Number of elements to send - including halos
            sendcounts[i] = ((i == 0 || i == size - 1) ? rows_i + 1 : rows_i + 2) * N;
            
            // Offset of rows to send - including halos
            displs_scatterv[i] = ((i == 0) ? 0 : base_offset - N); 
            base_offset += rows_i * N;
        }
    } 

    int my_sendcount = ((my_rank == 0 || my_rank == size - 1) ? base_rows + (my_rank < extra ? 1 : 0) + 1 : base_rows + (my_rank < extra ? 1 : 0) + 2) * N; // Number of elements to receive - including halos

    ///////////////////////////////////////////
    /////////////////////////////////////////// 
    ///////////////////////////////////////////

    double * recvbuf = malloc(my_sendcount * sizeof(double));

    ////////////////////////////////////////////
    // Compute recvcounts and displs for Gatherv
    ////////////////////////////////////////////

    int first_row, last_row;
    int *recvcounts;
    int *displs_gatherv;

    if (my_rank == 0) {
        recvcounts = malloc(size * sizeof(int));
        displs_gatherv = malloc(size * sizeof(int));

        for(int i=0; i<size; i++){
            int rows_i = base_rows + (i < extra ? 1 : 0);
            recvcounts[i] = rows_i * N; // senza halo
            displs_gatherv[i] = (i == 0) ? 0 : displs_gatherv[i-1] + recvcounts[i-1];
        }
    }

    int my_recvcount = base_rows * N + (my_rank < extra ? N : 0); // numero di righe da inviare - senza halo

    //////////////////////////////////////////
    //////////////////////////////////////////
    //////////////////////////////////////////

    int * sendbuf = malloc(my_recvcount * sizeof(int));

    // Matrix initialization
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
    }

    // Set first_row and last_row for local computation
    first_row = 1; // Discard upper halo
    last_row = my_sendcount/N - 2; // Discard last row of halo
    if (my_rank == 0) {
        first_row = 0; // First row without upper halo
    } else if (my_rank == size - 1) {
        last_row = my_sendcount/N - 1; // Last row without lower halo
    }

    MPI_Barrier(MPI_COMM_WORLD);
    start = MPI_Wtime();

    if (my_rank == 0) {
        MPI_Scatterv(
            A, 
            sendcounts, 
            displs_scatterv, 
            MPI_DOUBLE, 
            recvbuf, 
            sendcounts[0], 
            MPI_DOUBLE, 
            0, 
            MPI_COMM_WORLD
        );
    } else {
        MPI_Scatterv(
            NULL, 
            NULL, 
            NULL, 
            MPI_DOUBLE, 
            recvbuf, 
            my_sendcount, 
            MPI_DOUBLE, 
            0, 
            MPI_COMM_WORLD
        );
    }

    // Computer local sum
    for (int i = first_row; i < last_row + 1; i++){ // for each row
        for (int j = 0; j < N; j++){ // for each column
            double sum = 0.0;
            int count = 0;

            // Loop over the 3x3 neighborhood
            for (int di = -1; di <= 1; di++){
                for (int dj = -1; dj <= 1; dj++){
                    int ni = i + di;
                    int nj = j + dj;
                    if (ni >= 0 && ni < my_sendcount/N && nj >= 0 && nj < N){
                        sum += recvbuf[ni * N + nj];
                        count++;
                    }
                }
            }

            sendbuf[(i - first_row) * N + j] = (recvbuf[i * N + j] * count > sum) ? 1 : 0; // ottimizzazione
        }
    }    

    // Gather results
    if (my_rank == 0) {
        MPI_Gatherv(
            sendbuf, 
            recvcounts[0], 
            MPI_INT, 
            T, 
            recvcounts,
            displs_gatherv,
            MPI_INT, 
            0, 
            MPI_COMM_WORLD
        );
    } else {    
        MPI_Gatherv(
            sendbuf, 
            my_recvcount, 
            MPI_INT, 
            NULL, 
            NULL, 
            NULL, 
            MPI_INT, 
            0, 
            MPI_COMM_WORLD
        );
    }

    MPI_Barrier(MPI_COMM_WORLD); 
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

    // Cleanup
    if (my_rank == 0) {
        free(sendcounts);
        free(recvcounts);
        free(displs_scatterv);
        free(displs_gatherv);

        free(A);
        free(T);
    }

    free(recvbuf);
    free(sendbuf);

    MPI_Finalize();
    
    if (my_rank == 0) {
        printf("%lf\n", global_time);
    }

    return EXIT_SUCCESS;
}