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

#ifndef NANVIX_MPI_GROUP_H_
#define NANVIX_MPI_GROUP_H_

#include <nanvix/hal.h>
#include <mputil/object.h>
#include <mputil/proc.h>
#include <mpi.h>

/**
 * @brief Struct that defines a mpi_group.
 */
struct mpi_group_t
{
	object_t super;               /* Base object class.        */

	int my_rank;                  /* Local rank inside group.  */
	struct mpi_process_t **procs; /* Pointer to procs list.    */
	int size;                     /* Nr of processes in group. */
	struct mpi_group_t *parent;   /* Parent group pointer.     */
};

typedef struct mpi_group_t mpi_group_t;

/* Class declaration. */
OBJ_CLASS_DECLARATION(mpi_group_t);

/**
 * @brief Allocates a new group with @p group_size.
 *
 * @param group_size Number of processes that will be in the group.
 *
 * @returns Upon successful completion, the new group is returned.
 * A NULL pointer is returned instead.
 */
extern mpi_group_t * mpi_group_allocate(int group_size);

/**
 * @brief Allocates a new group with @p group_size and proc_list defined.
 *
 * @param procs      List of processes to be initialized in the group.
 * @param group_size Number of processes that will be in the group.
 *
 * @returns Upon successful completion, the new group is returned.
 * A NULL pointer is returned instead.
 */
extern mpi_group_t * mpi_group_allocate_w_procs(mpi_process_t ** procs, int group_size);

/**
 * @brief Frees the specified group.
 *
 * @param group The group to be freed.
 *
 * @returns Upon successful completion, zero is returned. A negative error code is
 * returned instead.
 */
extern int mpi_group_free(mpi_group_t ** group);

/**
 * @brief Gets the group size.
 *
 * @param group Group descriptor.
 *
 * @returns @p group size.
 */
static inline int mpi_group_size(mpi_group_t * group)
{
	return group->size;
}

/**
 * @brief Gets the local process rank inside the given group.
 *
 * @param group Group descriptor.
 *
 * @returns The local process rank inside the group.
 */
static inline int mpi_group_rank(mpi_group_t * group)
{
	return group->my_rank;
}

/**
 * @brief Initializes the groups submodule.
 *
 * @returns Upon successful completion, zero is returned. A
 * negative error code is returned instead.
 */
extern int mpi_group_init(void);

/**
 * @brief Finalizes the groups submodule.
 *
 * @returns Upon successful completion, zero is returned. A
 * negative error code is returned instead.
 */
extern int mpi_group_finalize(void);

#endif /* NANVIX_MPI_GROUP_H_ */
