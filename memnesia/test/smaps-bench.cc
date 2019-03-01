/*
 * Copyright (c) 2017-2019 Triad National Security, LLC
 *                         All rights reserved.
 *
 * This file is part of the mpimemu project. See the LICENSE file at the
 * top-level directory of this distribution.
 */

#include <cstdlib>
#include <cstdio>
#include <cassert>
#include <vector>

#include <limits.h>

#include "mpi.h"

#define MPICHK(rc) \
do { \
    assert(rc == MPI_SUCCESS); \
} while(0)

int
main(
    int argc,
    char **argv
) {
    int mpirc;
    int rank, numpe;

    mpirc = MPI_Init(&argc, &argv);
    MPICHK(mpirc);

    MPI_Comm_size(MPI_COMM_WORLD, &numpe);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    if (rank == 0) {
        printf(
            "# starting smaps benchmark with %d process%s.\n",
            numpe,
            numpe > 1 ? "es" : ""
        );
    }

    int n_trials = 100;
    double *times = (double *)calloc(n_trials, sizeof(double));

    for (int t = 0; t < n_trials; ++t) {
        static const char *f_name = "/proc/self/smaps";
        double start = MPI_Wtime();
        FILE *smapsf = fopen(f_name, "r");
        if (!smapsf) {
            perror("fopen");
            exit(EXIT_FAILURE);
        }
        char lbuff[PATH_MAX];
        size_t gets_size = PATH_MAX - 1;
        FILE *dn = fopen("/dev/null", "w");
        while (fgets(lbuff, gets_size, smapsf)) {
            fprintf(dn, "%s", lbuff);
        }
        fclose(dn);
        fclose(smapsf);
        double end = MPI_Wtime();
        times[t] = end - start;
    }

    for (int i = 0; i < n_trials; ++i) {
        printf("%d: %lf s\n", rank, times[i]);
    }

    free(times);
    MPI_Finalize();

    return EXIT_SUCCESS;
}
