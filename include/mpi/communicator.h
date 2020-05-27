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

#ifndef NANVIX_MPI_COMMUNICATOR_H_
#define NANVIX_MPI_COMMUNICATOR_H_

#include <mputil/object.h>
#include <mpi/group.h>
#include <mpi/errhandler.h>
#include <mpi.h>

/**
 * @brief Struct that defines a mpi_communicator.
 */
struct mpi_communicator_t
{
	object_t super;                        /* Base object class.             */

	struct mpi_group_t *group;             /* Group associated with comm.    */
	int my_rank;                           /* Local rank inside comm.        */
	int pt2pt_cid;                         /* Point-to-point context ID.     */
	int coll_cid;                          /* Collective context ID.         */

	mpi_errhandler_t *error_handler;       /* Comm error handler.            */
	mpi_errhandler_type_t errhandler_type; /* Type of associated errhandler. */

	struct mpi_communicator_t *parent;     /* Parent communicator.           */
};

typedef struct mpi_communicator_t mpi_communicator_t;

/* Class declaration. */
OBJ_CLASS_DECLARATION(mpi_communicator_t);

/**
 * @brief Allocates a new communicator.
 *
 * @param group_size Communicator group size.
 *
 * @returns A pointer to the new allocated communicator.
 */
extern mpi_communicator_t * mpi_comm_allocate(int group_size);

/**
 * @brief Frees the specified communicator.
 *
 * @param comm Communicator to be freed.
 *
 * @returns Upon successful completion, zero is returned with
 * @p comm pointing to NULL. A negative error code is returned
 * instead, without modifications on @p comm.
 */
extern int mpi_comm_free(mpi_communicator_t **comm);

/**
 * @brief Gets the local process rank in @p comm.
 *
 * @param comm Target communicator.
 *
 * @returns The local process rank in the communicator.
 */
static inline int mpi_comm_rank(mpi_communicator_t * comm)
{
    return (comm->my_rank);
}

/**
 * @brief Gets the size of the communicator.
 *
 * @param comm Target communicator.
 *
 * @returns The number of processes that participates in @p comm.
 */
static inline int mpi_comm_size(mpi_communicator_t * comm)
{
    return (comm->group->size);
}

/**
 * @brief Gets the communicator point-to-point cid.
 *
 * @param comm Target communicator.
 *
 * @returns The communicator's point-to-point context ID.
 */
static inline int mpi_comm_get_pt2pt_cid(mpi_communicator_t* comm)
{
    return (comm->pt2pt_cid);
}

/**
 * @brief Gets the communicator collective cid.
 *
 * @param comm Target communicator.
 *
 * @returns The communicator's collective context ID.
 */
static inline int mpi_comm_get_coll_cid(mpi_communicator_t* comm)
{
    return (comm->coll_cid);
}

/**
 * @brief Extracts the group associated with the communicator.
 *
 * @param comm  The target communicator.
 * @param group A group pointer to reference the target group upon success.
 *
 * @returns Upon successful completion, MPI_SUCCESS is returned with the
 * group reference on @p group. An MPI_ERROR_CODE is returned instead with
 * group pointing to NULL.
 */
extern int mpi_comm_group(mpi_communicator_t *comm, mpi_group_t **group);

/**
 * @brief Initializes the communicators submodule.
 *
 * @returns Upon successful completion, zero is returned. A
 * negative error code is returned instead.
 */
extern int mpi_comm_init(void);

/**
 * @brief Finalizes the communicators submodule.
 *
 * @returns Upon successful completion, zero is returned. A
 * negative error code is returned instead.
 */
extern int mpi_comm_finalize(void);

#endif /* NANVIX_MPI_COMMUNICATOR_H_ */
