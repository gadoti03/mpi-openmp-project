#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <time.h>
#define DIM 12

int main(int argc, char* argv[])
{
    double start, end;
    int size, my_rank;
    int A[DIM],B[DIM], C[DIM], i ;
    
    if (argc!=2)
    { 
        printf("sintassi sbagliata -- %s n_proc", argv[0]);
        exit(1);
    }

    size=atoi(argv[1]); // il numero di processi viene passato come argomento
    if(size > DIM)
    {
        printf("il numero di processi %d è maggiore della dimensione %d.\n",size, DIM);
        exit(1);
    }
    // inizializzazione vettori:
    srand((unsigned int)time(NULL));
    for(i=0;i<DIM;i++){
        A[i]=rand()%100;
        B[i]=rand()%100;
    }
    start = omp_get_wtime(); //campionamento del tempo di inizio
    # pragma omp parallel num_threads(size) shared(A,B,C, size) private(i,my_rank)
    {
        my_rank=omp_get_thread_num();
        printf("thread %d di %d: inizio il calcolo...\n", my_rank, size);
        
        # pragma omp for // il lavoro del for viene suddiviso tra i thread del team
        for(i=0; i<DIM; i++)
        C[i]=A[i]+B[i];
    }
    end = omp_get_wtime();

    printf("[Master] Risultato C=A+B:\n");
    for(i=0; i<DIM; i++)
        printf("\t%d\n", C[i]);
    
        printf("Tempo di esecuzione: %lf\n", end-start);
    return EXIT_SUCCESS;
}