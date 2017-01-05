/**
 * Copyright (c) 2010-2017 Los Alamos National Security, LLC.
 *                         All rights reserved.
 *
 * This program was prepared by Los Alamos National Security, LLC at Los Alamos
 * National Laboratory (LANL) under contract No. DE-AC52-06NA25396 with the U.S.
 * Department of Energy (DOE). All rights in the program are reserved by the DOE
 * and Los Alamos National Security, LLC. Permission is granted to the public to
 * copy and use this software without charge, provided that this Notice and any
 * statement of authorship are reproduced on all copies. Neither the U.S.
 * Government nor LANS makes any warranty, express or implied, or assumes any
 * liability or responsibility for the use of this software.
 */

/**
 * @author Samuel K. Gutierrez - samuel@lanl.gov
 * found a bug? have an idea? please let me know.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "mmu_constants.h"
#include "mmu_conv_macros.h"
#include "mmu_args.h"
#include "mmu_mpi.h"

#include "mpi.h"

#include <stdlib.h>
#include <stdio.h>
#ifdef HAVE_STDBOOL_H
#include <stdbool.h>
#endif
#ifdef HAVE_NETDB_H
#include <netdb.h>
#endif
#ifdef HAVE_STRING_H
#include <string.h>
#endif
#ifdef HAVE_SYS_SOCKET_H
#include <sys/socket.h>
#endif
#ifdef HAVE_NETINET_IN_H
#include <netinet/in.h>
#endif
#ifdef HAVE_ARPA_INET_H
#include <arpa/inet.h>
#endif

/* ////////////////////////////////////////////////////////////////////////// */
static const char *
get_mpi_err_str(int mpirc)
{
    int len = 0;
    static char err_buf[MPI_MAX_ERROR_STRING];

    MPI_Error_string(mpirc, err_buf, &len);

    return err_buf;
}

/* ////////////////////////////////////////////////////////////////////////// */
static const char *
get_herr_str(int h_err)
{
    static char *err_str;

    switch (h_err) {
        case HOST_NOT_FOUND:
            err_str = "the specified host is unknown.";
            break;
        case NO_ADDRESS:
            err_str = "the requested name is valid "
                      "but does not have an IP address.";
            break;
        case NO_RECOVERY:
            err_str = "a non-recoverable name server error occurred.";
            break;
        case TRY_AGAIN:
            err_str = "a temporary error occurred on an "
                      "authoritative name server.  Try again later.";
            break;
        default:
            err_str = "an unknown error occurred.";
            break;
    }

    return err_str;
}

#if MPI_VERSION < 3
/* ////////////////////////////////////////////////////////////////////////// */
static int
get_netnum(mmu_mpi_t *m,
           unsigned long int *netnum)
{
    struct hostent *host;

    if (NULL == m || NULL == netnum) return MMU_FAILURE_INVALID_ARG;

    if (NULL == (host = gethostbyname(m->hostname))) {
        int err = h_errno;
        fprintf(stderr, MMU_ERR_PREFIX"gethostbyname error: h_errno: %d (%s)\n",
                err,  get_herr_str(err));
        return MMU_FAILURE;
    }
    /* htonl used here because nodes could be different architectures */
    *netnum = htonl(inet_network(inet_ntoa(*(struct in_addr *)host->h_addr)));

    return MMU_SUCCESS;
}

/* ////////////////////////////////////////////////////////////////////////// */
static int
cmp_uli_fn(const void *p1,
           const void *p2)
{
    return (*(unsigned long int *)p1 - *(unsigned long int *)p2);
}

/* ////////////////////////////////////////////////////////////////////////// */
static int
get_my_color(unsigned long int *netnums,
             int netnums_len,
             unsigned long int my_netnum,
             int *my_color)
{
    int i = 0;
    int node_i = 0;
    unsigned long int prev_num;

    if (NULL == netnums   || 0 == netnums_len || NULL == my_color) {
        return MMU_FAILURE_INVALID_ARG;
    }

    qsort(netnums, (size_t)netnums_len, sizeof(unsigned long int), cmp_uli_fn);
    prev_num = netnums[0];

    while (i < netnums_len && prev_num != my_netnum) {
        /* quickly cycle through netnums that we've already seen */
        while (netnums[i] == prev_num) {
            ++i;
        }
        ++node_i;
        prev_num = netnums[i];
    }
    /* we din't find our netnum - how did that happen? */
    if (i >= netnums_len) {
        fprintf(stderr, MMU_ERR_PREFIX"could not determine my color.\n");
        return MMU_FAILURE;
    }
    *my_color = node_i;

    return MMU_SUCCESS;
}

/* ////////////////////////////////////////////////////////////////////////// */
static int
all_exchange_netnums(mmu_mpi_t *m,
                     unsigned long int my_netnum,
                     unsigned long int **all_netnums,
                     int *all_netnums_len)
{
    int rc = MMU_SUCCESS, mpirc;
    unsigned long int *tmp_netnums = NULL;

    if (NULL == m || NULL == all_netnums) return MMU_FAILURE_INVALID_ARG;

    if (NULL == (tmp_netnums = calloc(m->num_ranks, sizeof(*tmp_netnums)))) {
        MMU_OOR_COMPLAIN();
        return MMU_FAILURE_OOR;
    }
    if (MPI_SUCCESS != (mpirc = MPI_Allgather(&my_netnum, 1, MPI_UNSIGNED_LONG,
                                              tmp_netnums, 1, MPI_UNSIGNED_LONG,
                                              MPI_COMM_WORLD))) {
        fprintf(stderr, MMU_ERR_PREFIX"MPI_Allgather failure: %d (%s)\n", mpirc,
                get_mpi_err_str(mpirc));
        rc = MMU_FAILURE_MPI;
        goto out;
    }
    *all_netnums = tmp_netnums;
    *all_netnums_len = m->num_ranks;

out:
    if (MMU_SUCCESS != rc) {
        /* only free this in an error path */
        if (NULL != tmp_netnums) free(tmp_netnums);
        tmp_netnums = NULL;
    }

    return rc;
}

/* ////////////////////////////////////////////////////////////////////////// */
static int
set_locality_info(mmu_mpi_t *m)
{
    unsigned long int my_netnum = 0;
    unsigned long int *all_netnums = NULL;
    char *bad_func = NULL;
    int rc = MMU_FAILURE, mpirc = MPI_ERR_UNKNOWN, num_netnums, my_color;
    /* only smp rank 0 will contribue to the sum */
    int num_nodes_contribution = 0;

    if (NULL == m) return MMU_FAILURE_INVALID_ARG;

    if (MMU_SUCCESS != (rc = get_netnum(m, &my_netnum))) {
        fprintf(stderr, MMU_ERR_PREFIX"get_netnum failure.\n");
        return rc;
    }
    if (MMU_SUCCESS != (rc =
        all_exchange_netnums(m, my_netnum, &all_netnums, &num_netnums))) {
        fprintf(stderr, MMU_ERR_PREFIX"all_exchange_netnums failure.\n");
        /* rc already set */
        goto out;
    }
    if (MMU_SUCCESS != (rc =
        get_my_color(all_netnums, num_netnums, my_netnum, &my_color))) {
        fprintf(stderr, MMU_ERR_PREFIX"get_my_color failure.\n");
        /* rc already set */
        goto out;
    }
    /* create smp (node) communicator that contains processes that are located
     * on the same node */
    if (MPI_SUCCESS != (mpirc =
        MPI_Comm_split(MPI_COMM_WORLD, my_color, m->rank, &m->smp_comm))) {
        bad_func = "MPI_Comm_split";
        goto out;
    }
    m->free_smp_comm = true;
    /* now we can set smp (local node) information */
    if (MPI_SUCCESS != (mpirc = MPI_Comm_size(m->smp_comm, &m->num_smp_ranks))){
        bad_func = "MPI_Comm_size";
        goto out;
    }
    if (MPI_SUCCESS != (mpirc = MPI_Comm_rank(m->smp_comm, &m->smp_rank))) {
        bad_func = "MPI_Comm_rank";
        goto out;
    }
    /* now create the worker comm based on smp_rank */
    if (MPI_SUCCESS != (mpirc = MPI_Comm_split(MPI_COMM_WORLD, m->smp_rank,
                                               m->rank, &m->worker_comm))) {
        bad_func = "MPI_Comm_split";
        goto out;
    }
    m->free_worker_comm = true;
    if (MPI_SUCCESS != (mpirc = MPI_Comm_size(m->worker_comm,
                                              &m->num_workers))) {
        bad_func = "MPI_Comm_size";
        goto out;
    }
    if (MPI_SUCCESS != (mpirc = MPI_Comm_rank(m->worker_comm,
                                              &m->worker_rank))) {
        bad_func = "MPI_Comm_rank";
        goto out;
    }
    /* calculate number of nodes in our allocation */
    if (mmu_mpi_smp_rank_zero(m)) {
        num_nodes_contribution = 1;
    }
    if (MPI_SUCCESS != (mpirc = MPI_Allreduce(&num_nodes_contribution,
                                              &m->num_nodes, 1, MPI_INT,
                                              MPI_SUM, MPI_COMM_WORLD))) {
        bad_func = "MPI_Allreduce";
        goto out;
    }

out:
    if (MPI_SUCCESS != mpirc) {
        fprintf(stderr, MMU_ERR_PREFIX"%s failure: %d (%s)\n", bad_func, mpirc,
                get_mpi_err_str(mpirc));
        rc = MMU_FAILURE_MPI;
    }
    if (NULL != all_netnums) free(all_netnums);

    return rc;
}

#else /* MPI_VERSION < 3 */

static int
set_locality_info(mmu_mpi_t *m)
{
    char *bad_func = NULL;
    int rc = MMU_SUCCESS, mpirc = MPI_ERR_UNKNOWN;
    /* only smp rank 0 will contribue to the sum */
    int num_nodes_contribution = 0;

    if (NULL == m) return MMU_FAILURE_INVALID_ARG;

    /* create smp (node) communicator that contains processes that are located
     * on the same node */
    if (MPI_SUCCESS != (mpirc =
        MPI_Comm_split_type(MPI_COMM_WORLD, MPI_COMM_TYPE_SHARED, 0, MPI_INFO_NULL, &m->smp_comm))) {
        bad_func = "MPI_Comm_split";
        goto out;
    }
    m->free_smp_comm = true;
    /* now we can set smp (local node) information */
    if (MPI_SUCCESS != (mpirc = MPI_Comm_size(m->smp_comm, &m->num_smp_ranks))){
        bad_func = "MPI_Comm_size";
        goto out;
    }
    if (MPI_SUCCESS != (mpirc = MPI_Comm_rank(m->smp_comm, &m->smp_rank))) {
        bad_func = "MPI_Comm_rank";
        goto out;
    }
    /* now create the worker comm based on smp_rank */
    if (MPI_SUCCESS != (mpirc = MPI_Comm_split(MPI_COMM_WORLD, m->smp_rank,
                                               m->rank, &m->worker_comm))) {
        bad_func = "MPI_Comm_split";
        goto out;
    }
    m->free_worker_comm = true;
    if (MPI_SUCCESS != (mpirc = MPI_Comm_size(m->worker_comm,
                                              &m->num_workers))) {
        bad_func = "MPI_Comm_size";
        goto out;
    }
    if (MPI_SUCCESS != (mpirc = MPI_Comm_rank(m->worker_comm,
                                              &m->worker_rank))) {
        bad_func = "MPI_Comm_rank";
        goto out;
    }
    /* calculate number of nodes in our allocation */
    if (mmu_mpi_smp_rank_zero(m)) {
        num_nodes_contribution = 1;
    }
    if (MPI_SUCCESS != (mpirc = MPI_Allreduce(&num_nodes_contribution,
                                              &m->num_nodes, 1, MPI_INT,
                                              MPI_SUM, MPI_COMM_WORLD))) {
        bad_func = "MPI_Allreduce";
        goto out;
    }

out:
    if (MPI_SUCCESS != mpirc) {
        fprintf(stderr, MMU_ERR_PREFIX"%s failure: %d (%s)\n", bad_func, mpirc,
                get_mpi_err_str(mpirc));
        rc = MMU_FAILURE_MPI;
    }

    return rc;
}
#endif

/* ////////////////////////////////////////////////////////////////////////// */
static int
world_root_verify_smp_size(const mmu_mpi_t *m)
{
    int smp_size[2] = {m->num_smp_ranks, -m->num_smp_ranks};
    int mpirc;

    if (MPI_SUCCESS != (mpirc =
        MPI_Allreduce (MPI_IN_PLACE, smp_size, 2, MPI_INT, MPI_MAX, MPI_COMM_WORLD))) {
        fprintf(stderr, MMU_ERR_PREFIX"MPI_Allreduce failure: %d (%s)\n", mpirc,
                get_mpi_err_str(mpirc));
        return MMU_FAILURE_MPI;
    }

    if (smp_size[0] != -smp_size[1]) {
        return MMU_FAILURE_PPN_DIFFERS;
    }

    return MMU_SUCCESS;
}

/* ////////////////////////////////////////////////////////////////////////// */
int
mmu_mpi_construct(mmu_mpi_t **m)
{
    mmu_mpi_t *tmp = NULL;

    if (NULL == m) return MMU_FAILURE_INVALID_ARG;

    if (NULL == (tmp = (mmu_mpi_t *)calloc(1, sizeof(*tmp)))) {
        MMU_OOR_COMPLAIN();
        *m = NULL;
        return MMU_FAILURE_OOR;
    }
    (void)memset(tmp->hostname, '\0', MMU_MPI_MAX_PROCESSOR_NAME);
    *m = tmp;

    return MMU_SUCCESS;
}

/* ////////////////////////////////////////////////////////////////////////// */
int
mmu_mpi_destruct(mmu_mpi_t **m)
{
    mmu_mpi_t *tmp = NULL;

    if (NULL == m) return MMU_FAILURE_INVALID_ARG;

    tmp = *m;
    if (NULL != tmp) {
        free(tmp);
        tmp = NULL;
    }

    return MMU_SUCCESS;
}

/* ////////////////////////////////////////////////////////////////////////// */
const char *
mmu_mpi_get_version_str(const mmu_mpi_t *m)
{
    static char vbuf[64];

    (void)memset(vbuf, '\0', sizeof(vbuf));
    (void)snprintf(vbuf, sizeof(vbuf), "%d.%d",
                   m->version.version, m->version.subversion);

    return vbuf;
}

/* ////////////////////////////////////////////////////////////////////////// */
int
mmu_mpi_init(mmu_mpi_t *m,
             int argc,
             char **argv)
{
    int mpirc = MPI_ERR_UNKNOWN, rc = MMU_SUCCESS, tmp = 0;
    char *bad_func = NULL;

    if (NULL == m ) return MMU_FAILURE_INVALID_ARG;

    if (MPI_SUCCESS != (mpirc = MPI_Init(&argc, &argv))) {
        bad_func = "MPI_Init";
        goto out;
    }
    if (MPI_SUCCESS != (mpirc = MPI_Comm_rank(MPI_COMM_WORLD, &m->rank))) {
        bad_func = "MPI_Comm_rank";
        goto out;
    }
    if (MPI_SUCCESS != (mpirc = MPI_Comm_size(MPI_COMM_WORLD, &m->num_ranks))) {
        bad_func = "MPI_Comm_size";
        goto out;
    }
    if (MPI_SUCCESS != (mpirc = MPI_Get_processor_name(m->hostname, &tmp))) {
        bad_func = "MPI_Get_processor_name";
        goto out;
    }
    if (MPI_SUCCESS != (mpirc = MPI_Get_version(&m->version.version,
                                                &m->version.subversion))) {
        bad_func = "MPI_Get_version";
        goto out;
    }
    if (MMU_SUCCESS != (rc = set_locality_info(m))) {
        bad_func = "set_locality_info";
        goto out;
    }
    /* now determine if everyone has the same amount of processors per node */
    if (MMU_SUCCESS != (rc = world_root_verify_smp_size(m))) {
        /* don't set bad_func here, we don't want to have n error messages */
        rc = MMU_FAILURE_PPN_DIFFERS;
        goto out;
    }

out:
    if (NULL != bad_func) {
        /* was this an mpi error? */
        if (MPI_SUCCESS != mpirc) {
            fprintf(stderr, MMU_ERR_PREFIX"%s failure: rc: %d (%s)\n", bad_func,
                    mpirc, get_mpi_err_str(mpirc));
            rc = MMU_FAILURE_MPI;
        }
        else {
            fprintf(stderr, MMU_ERR_PREFIX"%s failure.\n", bad_func);
        }
    }

    return rc;
}

/* ////////////////////////////////////////////////////////////////////////// */
int
mmu_mpi_finalize(mmu_mpi_t *m)
{
    int rc;

    if (NULL == m) return MMU_FAILURE_INVALID_ARG;

    if (m->free_smp_comm) {
        if (MPI_SUCCESS != (rc = MPI_Comm_free(&m->smp_comm))) {
            fprintf(stderr, MMU_ERR_PREFIX"MPI_Comm_free failure: rc: %d (%s)\n",
                    rc, get_mpi_err_str(rc));
            /* don't return here so we can call MPI_Finalize */
        }
    }
    if (m->free_worker_comm) {
        if (MPI_SUCCESS != (rc = MPI_Comm_free(&m->worker_comm))) {
            fprintf(stderr, MMU_ERR_PREFIX"MPI_Comm_free failure: rc: %d (%s)\n",
                    rc, get_mpi_err_str(rc));
            /* don't return here so we can call MPI_Finalize */
        }
    }
    if (MPI_SUCCESS != (rc = MPI_Finalize())) {
        fprintf(stderr, MMU_ERR_PREFIX"MPI_Finalize failure: rc: %d (%s)\n", rc,
                get_mpi_err_str(rc));
        return MMU_FAILURE_MPI;
    }

    return MMU_SUCCESS;
}

/* ////////////////////////////////////////////////////////////////////////// */
bool
mmu_mpi_initialized(void)
{
    int initialized;

    (void)MPI_Initialized(&initialized);

    return (bool)initialized;
}

/* ////////////////////////////////////////////////////////////////////////// */
int
mmu_mpi_allreduce(void *sendbuf,
                  void *recvbuf,
                  int count,
                  MPI_Datatype datatype,
                  MPI_Op op,
                  MPI_Comm comm)
{
    int rc = MMU_SUCCESS, mpirc;

    mpirc = MPI_Allreduce(sendbuf, recvbuf, count, datatype, op, comm);
    if (MPI_SUCCESS != mpirc) {
        fprintf(stderr, MMU_ERR_PREFIX"MPI_Allreduce failure: rc: %d (%s)\n",
                mpirc, get_mpi_err_str(mpirc));
        rc = MMU_FAILURE_MPI;
    }

    return rc;
}

/* ////////////////////////////////////////////////////////////////////////// */
int
mmu_mpi_comm_size(MPI_Comm comm,
                  int *size)
{
    int rc = MMU_SUCCESS, mpirc;

    if (NULL == size) return MMU_FAILURE_INVALID_ARG;

    mpirc = MPI_Comm_size(comm, size);
    if (MPI_SUCCESS != mpirc) {
        fprintf(stderr, MMU_ERR_PREFIX"MPI_Comm_size failure: rc: %d (%s)\n",
                mpirc, get_mpi_err_str(mpirc));
        rc = MMU_FAILURE_MPI;
    }

    return rc;
}

/* ////////////////////////////////////////////////////////////////////////// */
bool
mmu_mpi_world_rank_zero(const mmu_mpi_t *m)
{
    return (0 == m->rank);
}

/* ////////////////////////////////////////////////////////////////////////// */
bool
mmu_mpi_smp_rank_zero(const mmu_mpi_t *m)
{
    return (0 == m->smp_rank);
}

/* ////////////////////////////////////////////////////////////////////////// */
int
mmu_mpi_get_num_smp_ranks(const mmu_mpi_t *m)
{
    return m->num_smp_ranks;
}

/* ////////////////////////////////////////////////////////////////////////// */
int
mmu_mpi_get_comm_world_size(const mmu_mpi_t *m)
{
    return m->num_ranks;
}

/* ////////////////////////////////////////////////////////////////////////// */
int
mmu_mpi_get_num_nodes(const mmu_mpi_t *m)
{
    return m->num_nodes;
}

/* ////////////////////////////////////////////////////////////////////////// */
bool
mmu_mpi_with_workload(const mmu_mpi_t *m)
{
    return m->with_comm_workload;
}

/* ////////////////////////////////////////////////////////////////////////// */
/* mpi workloads */
/* ////////////////////////////////////////////////////////////////////////// */
static int
get_msg_size(void)
{
    static int i = 0;
    static int msg_sizes_in_b[] = {1, 512, 1024, 8192, 16384};
    size_t num_elems = sizeof(msg_sizes_in_b) / sizeof(msg_sizes_in_b[0]);

    return msg_sizes_in_b[i++ % num_elems];
}

/* ////////////////////////////////////////////////////////////////////////// */
static int
small_allreduce_max(const mmu_mpi_t *m)
{
    int mpirc = MPI_ERR_UNKNOWN;
    double send_buff = (double)m->rank, recv_buff = 0.0;
    char *func = NULL;

    mpirc = MPI_Allreduce(&send_buff, &recv_buff, 1, MPI_DOUBLE,
                          MPI_MAX, MPI_COMM_WORLD);
    if (MPI_SUCCESS != mpirc) {
        func = "MPI_Allreduce";
        goto mpierr;
    }
    /* yes, i do want to do it this way :-) */
    if (recv_buff != (double)(m->num_ranks - 1)) {
        fprintf(stderr, MMU_ERR_PREFIX"invalid result detected!\n");
        return MMU_FAILURE;
    }
    mpirc = MPI_Barrier(MPI_COMM_WORLD);
    if (MPI_SUCCESS != mpirc) {
        func = "MPI_Barrier";
        goto mpierr;
    }

    return MMU_SUCCESS;

mpierr:
    fprintf(stderr, MMU_ERR_PREFIX"%s failure: rc: %d (%s)\n",
            func, mpirc, get_mpi_err_str(mpirc));
    return MMU_FAILURE_MPI;
}

/* ////////////////////////////////////////////////////////////////////////// */
static int
txrx_chain(const mmu_mpi_t *m)
{
    int mpirc = MPI_ERR_UNKNOWN, recv_tag = 42, send_tag = recv_tag;
    int r_neighbor = 0, l_neighbor = 0;
    int buff_size = get_msg_size();
    char *send_buffp = NULL ,*recv_buffp = NULL, *func = NULL;
    MPI_Status status;

    r_neighbor = (m->rank + 1) % m->num_ranks;
    l_neighbor = (m->rank + m->num_ranks - 1) % m->num_ranks;

    send_buffp = calloc(buff_size, sizeof(*send_buffp));
    recv_buffp = calloc(buff_size, sizeof(*recv_buffp));
    if (NULL == send_buffp || NULL == recv_buffp) {
        MMU_OOR_COMPLAIN();
        return MMU_FAILURE_OOR;
    }

    mpirc = MPI_Sendrecv(send_buffp, buff_size,
                         MPI_CHAR, r_neighbor, send_tag,
                         recv_buffp, buff_size, MPI_CHAR,
                         l_neighbor, recv_tag, MPI_COMM_WORLD,
                         &status);
    if (MPI_SUCCESS != mpirc) {
        func = "MPI_Sendrecv";
        goto mpierr;
    }

    mpirc = MPI_Sendrecv(send_buffp, buff_size,
                         MPI_CHAR, l_neighbor, send_tag,
                         recv_buffp, buff_size, MPI_CHAR,
                         r_neighbor, recv_tag, MPI_COMM_WORLD,
                         &status);
    if (MPI_SUCCESS != mpirc) {
        func = "MPI_Sendrecv";
        goto mpierr;
    }

    free(send_buffp);
    free(recv_buffp);

    return MMU_SUCCESS;
mpierr:
    fprintf(stderr, MMU_ERR_PREFIX"%s failure: rc: %d (%s)\n",
            func, mpirc, get_mpi_err_str(mpirc));
    return MMU_FAILURE_MPI;
}

/* ////////////////////////////////////////////////////////////////////////// */
int
workerxchange(const mmu_mpi_t *m)
{
    int mpirc = MPI_ERR_UNKNOWN, send_tag = 13, recv_tag = send_tag;
    int r_neighbor = 0, l_neighbor = 0;
    int i = 0, j = 0;
    char *func = NULL;
    char *send_buffp = NULL, *recv_buffp = NULL;
    int buff_size = get_msg_size();
    MPI_Status status;

    send_buffp = calloc(buff_size, sizeof(*send_buffp));
    recv_buffp = calloc(buff_size, sizeof(*recv_buffp));
    if (NULL == send_buffp || NULL == recv_buffp) {
        MMU_OOR_COMPLAIN();
        return MMU_FAILURE_OOR;
    }

    for (i = 1; i <= 3; ++i) {
        r_neighbor = (m->worker_rank + i) % m->num_workers;
        l_neighbor = m->worker_rank;

        for (j = 0; j < i; ++j) {
            --l_neighbor;
            if (l_neighbor < 0) {
                l_neighbor = m->num_workers - 1;
            }
        }

        mpirc = MPI_Sendrecv(send_buffp, buff_size,
                             MPI_CHAR, l_neighbor, send_tag,
                             recv_buffp, buff_size, MPI_CHAR,
                             r_neighbor, recv_tag, m->worker_comm,
                             &status);
        if (MPI_SUCCESS != mpirc) {
            func = "MPI_Sendrecv";
            goto mpierr;
        }
        mpirc = MPI_Sendrecv(send_buffp, buff_size,
                             MPI_CHAR, r_neighbor, send_tag,
                             recv_buffp, buff_size, MPI_CHAR,
                             l_neighbor, recv_tag, m->worker_comm,
                             &status);
        if (MPI_SUCCESS != mpirc) {
            func = "MPI_Sendrecv";
            goto mpierr;
        }
    }

    free(send_buffp);
    free(recv_buffp);

    return MMU_SUCCESS;
mpierr:
    fprintf(stderr, MMU_ERR_PREFIX"%s failure: rc: %d (%s)\n",
            func, mpirc, get_mpi_err_str(mpirc));
    return MMU_FAILURE_MPI;
}

/* ////////////////////////////////////////////////////////////////////////// */
int
mmu_mpi_work(const mmu_mpi_t *m)
{
    int rc = MMU_FAILURE;

    if (NULL == m) return MMU_FAILURE_INVALID_ARG;

    if (MMU_SUCCESS != (rc = small_allreduce_max(m))) {
        return rc;
    }
    if (MMU_SUCCESS != (rc = txrx_chain(m))) {
        return rc;
    }
    if (MMU_SUCCESS != (rc = workerxchange(m))) {
        return rc;
    }

    return rc;
}

/* ////////////////////////////////////////////////////////////////////////// */
int
mmu_mpi_enable_workload(mmu_mpi_t *m, bool enable)
{
    if (NULL == m) return MMU_FAILURE_INVALID_ARG;

    m->with_comm_workload = enable;

    return MMU_SUCCESS;
}
