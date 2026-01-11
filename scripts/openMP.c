#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main(int argc, char* argv[]) {
    if(argc < 3) {
        printf("Usage: %s N P\n", argv[0]);
        return 1;
    }

    int N = atoi(argv[1]);
    int P = atoi(argv[2]);

    if (P > N) {
        printf("Number of threads %d is greater than N=%d\n", P, N);
        return EXIT_FAILURE;
    }

    // Allocazione memoria continua
    double *A = malloc(N * N * sizeof(double));
    int *T = malloc(N * N * sizeof(int));
    if(!A || !T) { perror("malloc"); return EXIT_FAILURE; }

    // Inizializzazione matrice
    srand((unsigned int)time(NULL));
    for(int i=0;i<N;i++)
        for(int j=0;j<N;j++){
            A[i*N + j] = ((double)rand() / RAND_MAX) * 100.0;
            T[i*N + j] = 0;
        }

    double start = omp_get_wtime();

    #pragma omp parallel for schedule(static) num_threads(P)
    for(int i=0;i<N;i++){
        for(int j=0;j<N;j++){
            double sum = 0.0;
            int count = 0;

            // Loop sulle celle vicine
            for(int di=-1;di<=1;di++){
                for(int dj=-1;dj<=1;dj++){
                    int ni = i + di;
                    int nj = j + dj;
                    if(ni >= 0 && ni < N && nj >= 0 && nj < N){
                        sum += A[ni*N + nj];
                        count++;
                    }
                }
            }

            T[i*N + j] = (A[i*N + j] * count > sum) ? 1 : 0;
        }
    }

    double end = omp_get_wtime();
    printf("%lf\n", end - start);

    // Deallocazione
    free(A);
    free(T);

    return EXIT_SUCCESS;
}
