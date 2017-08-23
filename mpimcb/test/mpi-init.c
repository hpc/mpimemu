#include "mpi.h"
#include <unistd.h>

int
main(int argc, char **argv)
{
    MPI_Init(&argc, &argv);
    sleep(1000);
    MPI_Finalize();
    //
    return 0;
}
