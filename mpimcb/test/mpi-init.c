#include "mpi.h"
#include <stdlib.h>
#include <stdio.h>

int
main(int argc, char **argv)
{
    MPI_Init(&argc, &argv);
    //
    char *foo = (char *)malloc(8 * sizeof(*foo));
    free(foo);
    char *foo1 = (char *)malloc(8 * sizeof(*foo1));
    char *foo2 = (char *)malloc(8 * sizeof(*foo2));
    char *foo3 = (char *)calloc(8, sizeof(*foo3));
    char *foo4 = (char *)realloc(foo3, 16 * sizeof(*foo3));
    void *foo5 = NULL;
    (void)posix_memalign(&foo5, 16, 16);
    free(foo1);
    free(foo2);
    free(foo4);
    free(foo5);
    //
    MPI_Finalize();
    //
    return 0;
}
