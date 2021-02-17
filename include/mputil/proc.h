/*
 * MIT License
 *
 * Copyright(c) 2011-2020 The Maintainers of Nanvix
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef NANVIX_PROCESS_H_
#define NANVIX_PROCESS_H_

#include <nanvix/limits.h>
#include <nanvix/config.h>
#include <mputil/object.h>

/**
 * @brief Defines the number of active nodes.
 */
#ifndef MPI_NODES_NR
#define MPI_NODES_NR NANVIX_PROC_MAX
#endif

/**
 * @brief Defines the number of active MPI_PROCESSES.
 */
#ifndef MPI_PROCESSES_NR
#define MPI_PROCESSES_NR (MPI_NODES_NR * 2)
#endif

#define MPI_PROCS_PER_CLUSTER_MAX ((MPI_PROCESSES_NR / MPI_NODES_NR) +                \
                                   ((MPI_PROCESSES_NR % MPI_NODES_NR == 0) ? 0 : 1)   \
                                  )

/**
 * @brief Base compensation for clusters know their local mpi_id.
 */
#define MPI_PROCESSES_COMPENSATION SPAWNERS_NUM

/**
 * @brief Compensation for NoC nodes reserved for spawners.
 */
#define MPI_NODES_COMPENSATION (PROCESSOR_NOC_IONODES_NUM / PROCESSOR_IOCLUSTERS_NUM)

/**
 * @brief Struct that defines a dynamic pointer array.
 */
struct mpi_process_t
{
	object_t super; /* Base object class.    */

	char name[NANVIX_PROC_NAME_MAX];
	int pid;        /* Process ID.           */
	int tid;        /* Vinculated thread ID. */
	int inbox;
	int inportal;
};

typedef struct mpi_process_t mpi_process_t;

/* Class declaration. */
OBJ_CLASS_DECLARATION(mpi_process_t);

/**
 * @brief Gets the process name.
 *
 * @param proc Target process.
 *
 * @returns The process symbolic name.
 */
static inline const char * process_name(mpi_process_t *proc)
{
	return (proc->name);
}

/**
 * @brief Gets reference pointer to the current MPI process.
 *
 * @returns Pointer to the local process descriptor.
 */
extern mpi_process_t * curr_mpi_proc(void);

/**
 * @brief Gets the current process index in the local cluster.
 *
 * @returns The index of the current MPI Process in the local processes
 * table.
 */
extern int curr_mpi_proc_index(void);

/**
 * @brief Gets reference pointer to the current MPI process.
 *
 * @returns Pointer to the local process descriptor.
 */
static inline int curr_mpi_proc_inbox(void)
{
	return (curr_mpi_proc()->inbox);
}

/**
 * @brief Gets reference pointer to the current MPI process.
 *
 * @returns Pointer to the local process descriptor.
 */
static inline int curr_mpi_proc_inportal(void)
{
	return (curr_mpi_proc()->inportal);
}

/**
 * @brief Checks if the running MPI process is the master of the current cluster.
 *
 * @returns Returns ZERO if the running process is the master one. A NON-ZERO value
 * is returned instead.
 */
extern int curr_proc_is_master(void);

/**
 * @brief Allocates a new process for @p nodeid.
 *
 * @returns Upon successful completion, PID of the new
 * process in processes_list table is returned. A negative
 * MPI error code is returned instead.
 */
extern int process_allocate(void);

/**
 * @brief Returns a processes list containing all active processes.
 *
 * @param size Size variable to hold the list size info.
 *
 * @returns Upon successful completion, a pointer to the procs list
 * is returned. A NULL pointer is returned instead.
 *
 * @note The involved processes refcount is not updated. It is caller's
 * responsability to ensure correctness.
 */
extern mpi_process_t ** mpi_proc_world_list(int *size);

/**
 * @brief Returns a processes list containing only the local process.
 *
 * @param size Size variable to hold the list size info.
 *
 * @returns Upon successful completion, a pointer to the procs list
 * is returned. A NULL pointer is returned instead.
 *
 * @note The involved processes refcount is not updated. It is caller's
 * responsability to ensure correctness.
 */
extern mpi_process_t ** mpi_proc_self_list(int *size);

/**
 * @brief Gets the number of active processes.
 *
 * @returns The number of active processes.
 */
extern int mpi_proc_count(void);

/**
 * @brief Waits on the MPI std_fence. (local processes)
 *
 * @returns Upon successful completion, zero is returned. A negative
 * error code is returned instead.
 */
extern int mpi_std_fence(void);

/**
 * @brief Waits on the MPI std_barrier. (local and remote processes)
 *
 * @returns Upon successful completion, zero is returned. A negative
 * error code is returned instead.
 */
extern int mpi_std_barrier(void);

/**
 * @brief Initializes the process internal structures (thread specific).
 *
 * @returns Upon successful completion, zero is returned. A
 * negative MPI error code is returned instead.
 */
extern int mpi_local_proc_init(void);

/**
 * @brief Finalizes the process internal structures (thread specific).
 *
 * @returns Upon successful completion, zero is returned. A
 * negative MPI error code is returned instead.
 */
extern int mpi_local_proc_finalize(void);

/**
 * @brief Gets the number of locally present MPI processes.
 *
 * @returns The number of MPI processes that are locally present.
 */
extern int mpi_local_procs_nr(void);

#endif /* NANVIX_PROCESS_H_ */
