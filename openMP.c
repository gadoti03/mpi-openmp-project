#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main(int argc, char* argv[]) {
    if(argc < 3) {
        printf("Usage: %s N P\n", argv[0]);
        return 1;
    }

    double start, end;
    int N, P, chunk_size;
    double ** A;
    int ** T;

    N = atoi(argv[1]);
    P = atoi(argv[2]);

    if (P > N) {
        printf("il numero di processi %d è maggiore della dimensione %d\n", P, N);
        MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
    }

    // inizializzazione vettori
    srand((unsigned int)time(NULL));

    A = malloc(N * sizeof(double*));
    T = malloc(N * sizeof(int*));

    // inizializzazione vettori:
    for(int i=0;i<N;i++) {
        A[i] = malloc(N * sizeof(double));
        T[i] = malloc(N * sizeof(int));
        for(int j=0;j<N;j++) {
            A[i][j] = ((double)rand() / RAND_MAX) * 100.0;
            T[i][j] = 0; // inizializzo a zero
        }
    }

    start = omp_get_wtime(); // campionamento del tempo di inizio

    #pragma omp parallel for num_threads(P) schedule(static)
    for(int i = 0; i < N; i++){
        for(int j = 0; j < N; j++){
            double sum = 0.0;
            int count = 0;

            // loop sulle celle vicine
            for(int di = -1; di <= 1; di++){
                for(int dj = -1; dj <= 1; dj++){
                    int ni = i + di;
                    int nj = j + dj;
                    if(ni >= 0 && ni < N && nj >= 0 && nj < N){
                        sum += A[ni][nj];
                        count++;
                    }
                }
            }

            T[i][j] = (A[i][j] * count > sum) ? 1 : 0; // ottimizzazione
        }
    }

    end = omp_get_wtime(); // campionamento del tempo di fine

    printf("Tempo di esecuzione: %lf\n", end - start);

    // deallocazione memoria
    for(int i=0;i<N;i++){
        free(A[i]);
        free(T[i]);
    }
    free(A);
    free(T);

    return EXIT_SUCCESS;
}