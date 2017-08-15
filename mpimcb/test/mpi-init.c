#include "mpi.h"
#include <stdlib.h>
#include <stdio.h>

int
main(int argc, char **argv)
{
    MPI_Init(&argc, &argv);
    char *foo = malloc(8 * sizeof(*foo));
    free(foo);
    MPI_Finalize();
    return 0;
}
