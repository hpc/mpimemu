#include "mpi.h"
#include <unistd.h>
#include <stdlib.h>
#include <assert.h>

int
main(int argc, char **argv)
{
    static const int BIGB = 1024 * 1024;
    MPI_Init(&argc, &argv);

    int myid, numprocs;

    MPI_Comm_size(MPI_COMM_WORLD, &numprocs);
    MPI_Comm_rank(MPI_COMM_WORLD, &myid);

    static const int n_txrx = 10;

    for (int l = 0; l < n_txrx; ++l) {
        int *buffer = malloc(sizeof(int) * BIGB * numprocs);
        assert(buffer);
        int *buffer2 = malloc(sizeof(int) * BIGB * numprocs);
        assert(buffer2);

        int rc = MPI_Alltoall(
            buffer,
            BIGB,
            MPI_INT,
            buffer2,
            BIGB,
            MPI_INT,
            MPI_COMM_WORLD
        );

        assert(rc == MPI_SUCCESS);

        MPI_Barrier(MPI_COMM_WORLD);

        free(buffer);
        free(buffer2);
    }

    MPI_Barrier(MPI_COMM_WORLD);

    MPI_Finalize();
    //
    return 0;
}
