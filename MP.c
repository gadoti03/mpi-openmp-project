#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <time.h>

int main(int argc, char* argv[]) {
    if(argc != 2) {
        printf("Usage: %s N\n", argv[0]);
        return 1;
    }

    MPI_Init(&argc, &argv);

    ////////////// mettere vincoli su rapporto fra N e P /////////////////////////////////////////

    int size, my_rank, N;
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);

    N = atoi(argv[1]);

    double * A; // matrice master
    int * T; // matrice risultato
    double *my_A = NULL; // matrice locale
    int my_rows; // righe locali

    if (size > N) {
        printf("il numero di processi %d è maggiore della dimensione %d\n", size, N);
        MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
    }

    ///////////////////////////////////////////
    // calcolo sendcounts e displs per Scatterv
    ///////////////////////////////////////////

    int base_rows = N / size; // righe per processo
    int extra = N % size; // righe residue
    int base_offset = 0; // offset corrente

    int *sendcounts = NULL; // elementi per Scatterv
    int *displs = NULL; // offset per Scatterv

    sendcounts = malloc(size * sizeof(int));
    displs = malloc(size * sizeof(int));

    for(int i=0; i<size; i++){
        // assegno una riga extra ai primi 'extra' processi
        int rows_i = base_rows + (i < extra ? 1 : 0);
        
        // numero di righe da inviare - include le halo
        sendcounts[i] = ((i == 0 || i == size - 1) ? rows_i + 1 : rows_i + 2) * N;
        
        // offset delle righe da inviare - include le halo
        displs[i] = ((i == 0) ? 0 : base_offset - N); 
        base_offset += rows_i * N;
    }

    my_A = malloc(sendcounts[my_rank] * sizeof(double));

    printf("[Sono il processo %d di %d]\n", my_rank, size);

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

        MPI_Scatterv(
            A, 
            sendcounts, 
            displs, 
            MPI_DOUBLE, 
            my_A, 
            sendcounts[my_rank], 
            MPI_DOUBLE, 
            0, 
            MPI_COMM_WORLD
        );
    } else {
        MPI_Scatterv(
            NULL, 
            sendcounts, 
            displs, 
            MPI_DOUBLE, 
            my_A, 
            sendcounts[my_rank], 
            MPI_DOUBLE, 
            0, 
            MPI_COMM_WORLD
        );
    }

    printf("[processo %d] ricevuto %d elementi:\n", my_rank, sendcounts[my_rank]);
    for(int i = 0; i < sendcounts[my_rank]; i++){
        printf("%.1f ", my_A[i]);
        if ((i+1) % 5 == 0)  // ogni 5 elementi, vai a capo
            printf("\n");
    }
    if (sendcounts[my_rank] % 5 != 0)  // se l'ultimo gruppo < 5, vai a capo comunque
        printf("\n");
    fflush(stdout);

















    // Liberazione risorse
    free(my_A);
    free(sendcounts);
    free(displs);
    if(my_rank==0){
        free(A);
        free(T);
    }

    MPI_Finalize();
    return EXIT_SUCCESS;
}