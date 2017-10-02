#include "mpi.h"
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

int
main(int argc, char **argv)
{
    static const int SML = 512;
    static const int BIG = 1024 * 1024;
    int sizes[] = {SML, BIG};

    MPI_Init(&argc, &argv);

    int myid, numprocs;

    MPI_Comm_size(MPI_COMM_WORLD, &numprocs);
    MPI_Comm_rank(MPI_COMM_WORLD, &myid);

    static const int n_txrx = 50;

    for (int l = 0; l < n_txrx; ++l) {
        const int size = sizes[l % 2];
        int *buffer = malloc(sizeof(int) * size * numprocs);
        assert(buffer);
        int *buffer2 = malloc(sizeof(int) * size * numprocs);
        assert(buffer2);

        // Touch allocated pages.
        for (int i = 0; i < size * numprocs; ++i) {
            buffer[i] = myid;
            buffer2[i] = 0;
        }

        if (myid == 0) {
            printf("# alltoall size: %d B\n", (int)(size * sizeof(int)));
        }

        int rc = MPI_Alltoall(
            buffer,
            size,
            MPI_INT,
            buffer2,
            size,
            MPI_INT,
            MPI_COMM_WORLD
        );
        assert(rc == MPI_SUCCESS);

        free(buffer);
        free(buffer2);

        MPI_Barrier(MPI_COMM_WORLD);

    }

    MPI_Barrier(MPI_COMM_WORLD);

    MPI_Finalize();
    //
    return 0;
}
