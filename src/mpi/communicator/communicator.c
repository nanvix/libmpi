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

#include <nanvix/ulib.h>
#include <mpi/communicator.h>
#include <mpi/mpiruntime.h>
#include <mputil/communication.h>

PRIVATE void mpi_comm_construct(mpi_communicator_t *);
PRIVATE void mpi_comm_destruct(mpi_communicator_t *);

OBJ_CLASS_INSTANCE(mpi_communicator_t, &mpi_comm_construct, &mpi_comm_destruct,
                   sizeof(mpi_communicator_t));

/**
 * @brief Predefined communicators.
 */
mpi_communicator_t _mpi_comm_world;
mpi_communicator_t _mpi_comm_self;
mpi_communicator_t _mpi_comm_null;

/**
 * @brief Communicator constructor.
 */
PRIVATE void mpi_comm_construct(mpi_communicator_t * comm)
{
	uassert(comm != NULL);

	comm->group           = NULL;
	comm->pt2pt_cid       = MPI_UNDEFINED;
	comm->coll_cid        = MPI_UNDEFINED;
	comm->error_handler   = NULL;
	comm->errhandler_type = MPI_ERRHANDLER_TYPE_COMM;
	comm->parent          = NULL;
}

/**
 * @brief Communicator destructor.
 *
 * @note When cid allocation became dynamic, it should be released here.
 */
PRIVATE void mpi_comm_destruct(mpi_communicator_t * comm)
{
	uassert(comm != NULL);

	/* Release the associated group. */
	if (comm->group != NULL)
		OBJ_RELEASE(comm->group);

	/* Release the associated error handler. */
	if (comm->error_handler != NULL)
		OBJ_RELEASE(comm->error_handler);

	/* Release parent reference. */
	if (comm->parent != NULL)
		OBJ_RELEASE(comm->parent);
}

/**
 * @see mpi_comm_allocate() at communicator.h.
 */
PUBLIC mpi_communicator_t * mpi_comm_allocate(int group_size)
{
	mpi_communicator_t *new_comm;
	mpi_group_t *group;

	/* Bad group_size. */
	if (group_size < 0)
		return (NULL);

	/* Allocates a new communicator. */
	new_comm = OBJ_NEW(mpi_communicator_t);
	if (new_comm == NULL)
		return (NULL);

	/* Allocates a new group. */
	group = mpi_group_allocate(group_size);
	if (group == NULL)
	{
		OBJ_RELEASE(new_comm);
		return (NULL);
	}

	new_comm->group = group;

	return (new_comm);
}

/**
 * @see mpi_comm_free() at communicator.h.
 */
PUBLIC int mpi_comm_free(mpi_communicator_t **comm)
{
	/* Bad communicator. */
	if (*comm == NULL)
		return (-EINVAL);

	OBJ_RELEASE(comm);
	*comm = MPI_COMM_NULL;

	return (MPI_SUCCESS);
}

/**
 * @see mpi_comm_group() at communicator.h.
 */
PUBLIC int mpi_comm_group(mpi_communicator_t *comm, mpi_group_t **group)
{
	/* Bad communicator. */
	if (comm == NULL)
		return (MPI_ERR_COMM);

	/* Bad group receptor. */
	if (group == NULL)
		return (MPI_ERR_ARG);

	/* Increments the group reference counter. */
	OBJ_RETAIN(comm->group);
	*group = comm->group;

	return (MPI_SUCCESS);
}

/**
 * @todo Provide a detailed description.
 */
PUBLIC int mpi_comm_get_proc(mpi_communicator_t *comm, int rank, mpi_process_t **proc)
{
	int ret;

	/* Bad communicator. */
	if (comm == NULL)
		return (MPI_ERR_COMM);

	/* Bad proc receptor. */
	if (proc == NULL)
		return (MPI_ERR_ARG);

	/* Invalid rank. */
	if (!mpi_comm_peer_rank_is_valid(comm, rank))
		return (MPI_ERR_RANK);

	/* Gets the proc reference from within comm group. */
	ret = mpi_group_get_proc(comm->group, rank, proc);

	return (ret);
}

/**
 * @see mpi_comm_init() at communicator.h.
 *
 * @todo See necessity of defining a communicators array.
 * @todo See necessity of freeing pre-allocated comms in case of failure.
 */
PUBLIC int mpi_comm_init(void)
{
	mpi_group_t *group;

	/* Setup MPI_COMM_WORLD. */
	OBJ_CONSTRUCT(&_mpi_comm_world, mpi_communicator_t);
	group = OBJ_NEW(mpi_group_t);

	if ((group->procs = mpi_proc_world_list(&(group->size))) == NULL)
		return (MPI_ERR_NO_MEM);

	mpi_group_increment_proc_count(group);

	_mpi_comm_world.group         = group;
	_mpi_comm_world.pt2pt_cid     = 0;
	_mpi_comm_world.coll_cid      = 1;
	_mpi_comm_world.error_handler = MPI_ERRORS_ARE_FATAL;
	OBJ_RETAIN(_mpi_comm_world.error_handler);

	/* Setup MPI_COMM_SELF. */
	OBJ_CONSTRUCT(&_mpi_comm_self, mpi_communicator_t);
	group = OBJ_NEW(mpi_group_t);

	if ((group->procs = mpi_proc_self_list(&(group->size))) == NULL)
		return (MPI_ERR_NO_MEM);

	mpi_group_increment_proc_count(group);

	_mpi_comm_self.group         = group;
	_mpi_comm_self.pt2pt_cid     = 2;
	_mpi_comm_self.coll_cid      = MPI_UNDEFINED;
	_mpi_comm_self.error_handler = MPI_ERRORS_ARE_FATAL;
	OBJ_RETAIN(_mpi_comm_self.error_handler);

	/* Setup MPI_COMM_NULL. */
	OBJ_CONSTRUCT(&_mpi_comm_null, mpi_communicator_t);
	_mpi_comm_null.group         = MPI_GROUP_NULL;
	OBJ_RETAIN(_mpi_comm_null.group);
	_mpi_comm_null.error_handler = MPI_ERRORS_ARE_FATAL;
	OBJ_RETAIN(_mpi_comm_null.error_handler);
	
	return (MPI_SUCCESS);
}

/**
 * @see mpi_comm_finalize() at communicator.h.
 *
 * @todo If define a communicators list we should free it here.
 */
PUBLIC int mpi_comm_finalize(void)
{
	/* Grants that MPI_COMM_SELF was already destructed. */
	uassert(_mpi_state == MPI_STATE_FINALIZE_DESTRUCT_COMM_SELF);

	/* Destruct MPI_COMM_WORLD. */
	OBJ_DESTRUCT(&_mpi_comm_world);

	/* Destruct MPI_COMM_NULL. */
	OBJ_DESTRUCT(&_mpi_comm_null);

	return (MPI_SUCCESS);
}

/**
 * @see mpi_destruct_comm_self() at communicator.h.
 */
PUBLIC int mpi_destruct_comm_self(void)
{
	/* Destruct MPI_COMM_SELF. */
	OBJ_DESTRUCT(&_mpi_comm_self);

	return (MPI_SUCCESS);
}
