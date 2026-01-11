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

    MPI_Init(&argc, &argv);

    int size, my_rank, N;
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);

    N = atoi(argv[1]);

    double * A; // matrice master
    int * T; // matrice risultato

    int base_rows = N / size; // righe per processo
    int extra = N % size; // righe residue

    ///////////////////////////////////////////
    // calcolo sendcounts e displs per Scatterv
    ///////////////////////////////////////////

    int * sendcounts = malloc(size * sizeof(int));
    int * displs_scatterv = malloc(size * sizeof(int));

    int base_offset = 0; // offset corrente

    for(int i=0; i<size; i++){
        // assegno una riga extra ai primi 'extra' processi
        int rows_i = base_rows + (i < extra ? 1 : 0);
        
        // numero di righe da inviare - include le halo
        sendcounts[i] = ((i == 0 || i == size - 1) ? rows_i + 1 : rows_i + 2) * N;
        
        // offset delle righe da inviare - include le halo
        displs_scatterv[i] = ((i == 0) ? 0 : base_offset - N); 
        base_offset += rows_i * N;
    }

    ///////////////////////////////////////////
    /////////////////////////////////////////// 
    ///////////////////////////////////////////

    //////////////////////////////////////////
    // calcolo recvcounts e displs per Gatherv
    //////////////////////////////////////////

    int first_row, last_row;
    int *recvcounts = malloc(size * sizeof(int));
    int *displs_gatherv = malloc(size * sizeof(int));

    for(int i=0; i<size; i++){
        int rows_i = base_rows + (i < extra ? 1 : 0);
        recvcounts[i] = rows_i * N; // senza halo
        displs_gatherv[i] = (i == 0) ? 0 : displs_gatherv[i-1] + recvcounts[i-1];
    }

    //////////////////////////////////////////
    //////////////////////////////////////////
    //////////////////////////////////////////

    double * recvbuf = malloc(sendcounts[my_rank] * sizeof(double));

    // Inizializzazione matrice
    if (my_rank == 0) {
        // Alloca e inizializza matrice (in maniera continue)
        A = malloc(N * N * sizeof(double));
        T = malloc(N * N * sizeof(int));

        srand((unsigned int)time(NULL));
        
        for(int i = 0; i < N; i++){
            for(int j = 0; j < N; j++){
                A[i*N + j] = ((double)rand() / RAND_MAX) * 100.0;
                T[i*N + j] = 0;  // inizializzo a zero
            }
        }
    }

    start = MPI_Wtime();

    if (my_rank == 0) {
        MPI_Scatterv(
            A, 
            sendcounts, 
            displs_scatterv, 
            MPI_DOUBLE, 
            recvbuf, 
            sendcounts[my_rank], 
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
            sendcounts[my_rank], 
            MPI_DOUBLE, 
            0, 
            MPI_COMM_WORLD
        );
    }

    // calcolo somma locale (senza halo)
    int * sendbuf = malloc(recvcounts[my_rank] * sizeof(int));

    // setto righe iniziali e finali in base alla presenza di halo
    first_row = 1; // scarto la prima riga di halo
    last_row = sendcounts[my_rank]/N - 2; // scarto l'ultima riga di halo
    if (my_rank == 0) {
        first_row = 0; // prima riga senza halo sopra
    } else if (my_rank == size - 1) {
        last_row = sendcounts[my_rank]/N - 1; // ultima riga senza halo sotto
    }

    // calcolo somma locale
    for (int i = first_row; i < last_row + 1; i++){ // per ogni riga locale valida
        for (int j = 0; j < N; j++){ // per ogni colonna
            double sum = 0.0;
            int count = 0;

            // loop sulle celle vicine
            for (int di = -1; di <= 1; di++){
                for (int dj = -1; dj <= 1; dj++){
                    int ni = i + di;
                    int nj = j + dj;
                    if (ni >= 0 && ni < sendcounts[my_rank]/N && nj >= 0 && nj < N){
                        sum += recvbuf[ni * N + nj];
                        count++;
                    }
                }
            }

            sendbuf[(i - first_row) * N + j] = (recvbuf[i * N + j] * count > sum) ? 1 : 0; // ottimizzazione
        }
    }    

    // raccolta risultati
    MPI_Gatherv(
        sendbuf, 
        recvcounts[my_rank], 
        MPI_INT, 
        T, 
        recvcounts, 
        displs_gatherv, 
        MPI_INT, 
        0, 
        MPI_COMM_WORLD
    );

    MPI_Barrier(MPI_COMM_WORLD); 
    end = MPI_Wtime();

    /*
    // stampa risultato 
    if (my_rank == 0) {
        printf("\n[Master] Risultato T (matrice binaria):\n");
        for(int i=0;i<N;i++) {
            for(int j=0;j<N;j++) {
                printf("%d ", T[i*N + j]);
            }
            printf("\n");
        }
    }
    */

    // deallocazione memoria
    free(recvbuf);
    free(sendbuf);
    free(sendcounts);
    free(recvcounts);
    free(displs_scatterv);
    free(displs_gatherv);

    if (my_rank == 0) {
        free(A);
        free(T);
    }

    MPI_Finalize();

    end = MPI_Wtime();

    if (my_rank == 0) {
        printf("%lf\n", end - start);
    }

    return EXIT_SUCCESS;
}