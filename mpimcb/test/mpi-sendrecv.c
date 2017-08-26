#include "mpi.h"
#include <unistd.h>

int
main(int argc, char **argv)
{
    MPI_Init(&argc, &argv);

    int myid, numprocs, left, right;

    MPI_Comm_size(MPI_COMM_WORLD, &numprocs);
    MPI_Comm_rank(MPI_COMM_WORLD, &myid);

    static const int n_txrx = 256;
    for (int i = 0; i < n_txrx; ++i) {
        int buffer[10], buffer2[10];
        MPI_Status status;
 
 
        right = (myid + 1) % numprocs;
        left = myid - 1;
        if (left < 0) left = numprocs - 1;
 
        MPI_Sendrecv(
            buffer,
            10,
            MPI_INT,
            left,
            7,
            buffer2,
            10,
            MPI_INT,
            right,
            7,
            MPI_COMM_WORLD,
            &status
        );
    }

    MPI_Finalize();
    //
    return 0;
}
