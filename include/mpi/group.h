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

#include <mputil/object.h>
#include <mputil/proc.h>
#include <mpi.h>

/**
 * @brief Struct that defines a mpi_group.
 */
typedef struct mpi_group_t
{
	object_t super;               /* Base object class.        */

	struct mpi_process_t **procs; /* Pointer to procs list.    */
	int size;                     /* Nr of processes in group. */
	struct mpi_group_t *parent;   /* Parent group pointer.     */
} mpi_group_t;

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
 * @brief Checks if a group pointer is valid.
 *
 * @param group Target group.
 *
 * @returns Zero if the group is not valid, and a non-zero
 * value otherwise.
 */
static inline int mpi_group_is_valid(mpi_group_t* group)
{
    return ((group != NULL) && (group != MPI_GROUP_NULL));
}

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
 * @param rank  Returned rank holder.
 *
 * @returns Upon successful completion, MPI_SUCESS is returned.
 * Upon failure, a negative error code is returned instead.
 */
extern int mpi_group_rank(mpi_group_t * group, int *rank);

/**
 * @brief Gets the proc associated with rank @p rank.
 *
 * @param group Target group.
 * @param rank  Target rank.
 * @param proc  Pointer to hold the target process reference.
 *
 * @returns Upon successful completion, MPI_SUCCESS is returned and @p proc
 * points to the desired proc. Uṕon failure, an MPI Error code is returned with
 * @p proc pointing to NULL.
 */
extern int mpi_group_get_proc(mpi_group_t *group, int rank, mpi_process_t **proc);

/**
 * @brief Increments the refcount of each process in @p group.
 *
 * @param group Group of processes to be manipulated.
 */
extern void mpi_group_increment_proc_count(mpi_group_t *);

/**
 * @brief Decrements the refcount of each process in @p group.
 *
 * @param group Group of processes to be manipulated.
 */
extern void mpi_group_decrement_proc_count(mpi_group_t *);

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
