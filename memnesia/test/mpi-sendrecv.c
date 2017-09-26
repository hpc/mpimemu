#include "mpi.h"
#include <unistd.h>
#include <stdlib.h>

#define BIGB 4 * 1024 * 1024
#define SMLB 1024

int
main(int argc, char **argv)
{
    MPI_Init(&argc, &argv);

    int myid, numprocs, left, right;

    MPI_Comm_size(MPI_COMM_WORLD, &numprocs);
    MPI_Comm_rank(MPI_COMM_WORLD, &myid);

    static const int n_txrx = 1024;

    for (int i = 0; i < n_txrx; ++i) {
        int *buffer = malloc(sizeof(int) * BIGB);
        int *buffer2 = malloc(sizeof(int) * BIGB);

        right = (myid + 1) % numprocs;
        left = myid - 1;
        if (left < 0) left = numprocs - 1;

        MPI_Sendrecv(
            buffer,
            BIGB,
            MPI_INT,
            left,
            i,
            buffer2,
            BIGB,
            MPI_INT,
            right,
            i,
            MPI_COMM_WORLD,
            MPI_STATUS_IGNORE
        );
        free(buffer);
        free(buffer2);
    }

    MPI_Barrier(MPI_COMM_WORLD);

    for (int i = 0; i < n_txrx; ++i) {
        int buffer[SMLB], buffer2[SMLB];


        right = (myid + 1) % numprocs;
        left = myid - 1;
        if (left < 0) left = numprocs - 1;

        MPI_Sendrecv(
            buffer,
            SMLB,
            MPI_INT,
            right,
            i,
            buffer2,
            SMLB,
            MPI_INT,
            left,
            i,
            MPI_COMM_WORLD,
            MPI_STATUS_IGNORE
        );
    }

    MPI_Barrier(MPI_COMM_WORLD);

    MPI_Finalize();
    //
    return 0;
}
