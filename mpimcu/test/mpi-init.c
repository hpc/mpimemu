#include "mpi.h"
#include <unistd.h>
#include <stdlib.h>

int
main(int argc, char **argv)
{
    size_t alloc_size = 1024 * 1024 * 13;
    char *app_mem = (char *)malloc(alloc_size);
    for (size_t i = 0; i < alloc_size; ++i) {
        app_mem[i] = i % 256;
    }

    MPI_Init(&argc, &argv);
    MPI_Finalize();

    free(app_mem);
    //
    return 0;
}
