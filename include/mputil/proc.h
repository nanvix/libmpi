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

#include <nanvix/hal.h>
#include <mputil/object.h>

#define PROCESS_NAME_MAX_LENGTH 32

/**
 * @brief Struct that defines a dynamic pointer array.
 */
struct mpi_process_t
{
	object_t super; /* Base object class. */

	char name[PROCESS_NAME_MAX_LENGTH];
	int pid;        /* Process ID.        */
	int nodenum;    /* Process nodenum.   */
};

typedef struct mpi_process_t mpi_process_t;

/* Class declaration. */
OBJ_CLASS_DECLARATION(mpi_process_t);

/**
 * @brief Allocates a new process for @p nodeid.
 *
 * @param nodeid Process node number.
 *
 * @returns Upon successful completion, the index of the new
 * process in processes_list table is returned. A negative
 * error code is returned instead.
 */
extern int process_allocate(int nodeid);

/**
 * @brief Get the process nodenum.
 *
 * @param proc Process descriptor.
 *
 * @returns @p proc nodenum.
 */
static inline int process_nodenum(mpi_process_t *proc)
{
	return proc->nodenum;
}

/**
 * @brief Gets reference pointer to the local process.
 *
 * @returns Pointer to the local process descriptor.
 */
extern mpi_process_t * process_local(void);

/**
 * @brief Make a name lookup on the processes list.
 *
 * @param name Name of process to lookup.
 *
 * @returns UPon successful completion. a pointer to the process
 * descriptor is returned. Upon failure, a NULL pointer is returned
 * instead.
 *
 * @note If the name could not be found, a NULL pointer is returned.
 */
extern mpi_process_t * process_lookup(const char* name);

/**
 * @brief Gets the number of processes active.
 *
 * @returns The number of active processes.
 */
extern int mpi_proc_count(void);

/**
 * @brief Initializes the processes submodule.
 *
 * @returns Upon successful completion, zero is returned. A
 * negative error code is returned instead.
 */
extern int mpi_proc_init(void);

/**
 * @brief Finalizes the processes submodule.
 *
 * @returns Upon successful completion, zero is returned. A
 * negative error code is returned instead.
 */
extern int mpi_proc_finalize(void);

#endif /* NANVIX_PROCESS_H_ */
