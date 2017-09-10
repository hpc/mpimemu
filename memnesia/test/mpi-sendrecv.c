#include "mpi.h"
#include <unistd.h>

int
main(int argc, char **argv)
{
    MPI_Init(&argc, &argv);

    int myid, numprocs, left, right;

    MPI_Comm_size(MPI_COMM_WORLD, &numprocs);
    MPI_Comm_rank(MPI_COMM_WORLD, &myid);

    static const int n_txrx = 1024;

    for (int i = 0; i < n_txrx; ++i) {
        int buffer[100], buffer2[100];


        right = (myid + 1) % numprocs;
        left = myid - 1;
        if (left < 0) left = numprocs - 1;

        MPI_Sendrecv(
            buffer,
            100,
            MPI_INT,
            left,
            i,
            buffer2,
            100,
            MPI_INT,
            right,
            i,
            MPI_COMM_WORLD,
            MPI_STATUS_IGNORE
        );
    }

    MPI_Barrier(MPI_COMM_WORLD);

    for (int i = 0; i < n_txrx; ++i) {
        int buffer[10], buffer2[10];


        right = (myid + 1) % numprocs;
        left = myid - 1;
        if (left < 0) left = numprocs - 1;

        MPI_Sendrecv(
            buffer,
            10,
            MPI_INT,
            right,
            i,
            buffer2,
            10,
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
