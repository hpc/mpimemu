#include "mpi.h"
#include <stdlib.h>

int
main(int argc, char **argv)
{
    MPI_Init(&argc, &argv);
    char *foo = malloc(8 * sizeof(*foo));
    char *foo1 = malloc(16 * sizeof(*foo));
    free(foo);
    char *foo2 = malloc(10 * sizeof(*foo));
    free(foo1);
    free(foo2);
    MPI_Finalize();
    return 0;
}
