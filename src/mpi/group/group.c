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

#include <nanvix/hlib.h>
#include <nanvix/ulib.h>
#include <posix/errno.h>
#include <mpi/group.h>

PRIVATE void mpi_group_construct(mpi_group_t *);
PRIVATE void mpi_group_destruct(mpi_group_t *);

OBJ_CLASS_INSTANCE(mpi_group_t, &mpi_group_construct, &mpi_group_destruct, sizeof(mpi_group_t));

/**
 * @brief Predefined groups (for effects of comparation).
 */
mpi_group_t _mpi_group_empty;
mpi_group_t _mpi_group_null;

/**
 * @brief Group constructor.
 */
PRIVATE void mpi_group_construct(mpi_group_t * group)
{
	uassert(group != NULL);

    group->my_rank = -1;
    group->procs   = NULL;
    group->size    = -1;
    group->parent  = NULL;
}

/**
 * @brief Group destructor.
 */
PRIVATE void mpi_group_destruct(mpi_group_t * group)
{
    uassert(group != NULL);

    if (group->procs != NULL)
    {
    	mpi_group_decrement_proc_count(group);
    	ufree(group->procs);
    }

    if (group->parent != NULL)
    	OBJ_RELEASE(group->parent);
}

/**
 * @brief Increments the refcount of each process in @p group.
 *
 * @param group Group of processes to be manipulated.
 */
PUBLIC void mpi_group_increment_proc_count(mpi_group_t * group)
{
	uassert(group != NULL);
	uassert(group->size >= 0);

	if (group->size == 0)
		return;

	/* Traverse the processes list. */
	for (int i = 0; i < group->size; ++i)
		OBJ_RETAIN(group->procs[i]);
}

/**
 * @brief Decrements the refcount of each process in @p group.
 *
 * @param group Group of processes to be manipulated.
 */
PUBLIC void mpi_group_decrement_proc_count(mpi_group_t * group)
{
	uassert(group != NULL);
	uassert(group->size >= 0);

	if (group->size == 0)
		return;

	/* Traverse the processes list. */
	for (int i = 0; i < group->size; ++i)
		OBJ_RELEASE(group->procs[i]);
}

/**
 * @brief Allocates a new group with @p group_size.
 *
 * @param group_size Number of processes that will be in the group.
 *
 * @returns Upon successful completion, the new group is returned.
 * A NULL pointer is returned instead.
 */
PUBLIC mpi_group_t * mpi_group_allocate(int group_size)
{
	mpi_group_t * new_group;
	mpi_process_t ** procs;

	/* Bad group size. */
	if (group_size < 0)
		return (NULL);

	if (group_size == 0)
	{
		OBJ_RETAIN(&_mpi_group_empty);
		return (&_mpi_group_empty);
	}

	/* Allocates the processes list. */
	procs = (mpi_process_t **) ucalloc(group_size, sizeof(mpi_process_t *));
	if (procs == NULL)
		return (NULL);

	new_group = mpi_group_allocate_w_procs(procs, group_size);
	if (new_group == NULL)
		ufree(procs);
	
	return (new_group);
}

/**
 * @brief Allocates a new group with @p group_size and proc_list defined.
 *
 * @param procs      List of processes to be initialized in the group.
 * @param group_size Number of processes that will be in the group.
 *
 * @returns Upon successful completion, the new group is returned.
 * A NULL pointer is returned instead.
 */
PUBLIC mpi_group_t * mpi_group_allocate_w_procs(mpi_process_t ** procs, int group_size)
{
	mpi_group_t * new_group;

	/* Bad group size. */
	if (group_size < 0)
		return (NULL);

	/* Bad processes list. */
	if (procs == NULL)
		return (NULL);

	/* Allocates the new group. */
	if (group_size == 0)
	{
		new_group = &_mpi_group_empty;
		OBJ_RETAIN(new_group);
	}
	else
	{
		new_group = OBJ_NEW(mpi_group_t);
		if (new_group == NULL)
			return (NULL);

		/* Initializes the group attributes. */
		new_group->my_rank = MPI_UNDEFINED;
		new_group->procs   = procs;
		new_group->size    = group_size;
		new_group->parent  = NULL;
	}

	return (new_group);
}

/**
 * @brief Frees the specified group.
 *
 * @param group The group to be freed.
 *
 * @returns Upon successful completion, zero is returned and @p group
 * set to point to MPI_PROC_NULL. A negative error code is returned
 * instead.
 */
PUBLIC int mpi_group_free(mpi_group_t ** group)
{
	/* Bad group. */
	if (*group == NULL)
		return (-EINVAL);

	OBJ_RELEASE(group);
	*group = MPI_GROUP_NULL;

	return (0);
}

/**
 * @brief Sets the local process rank inside the given group.
 *
 * @param group Group descriptor.
 * @param proc  Local process descriptor.
 *
 * @note A NULL pointer in @p proc will set the group rank to MPI_UNDEFINED.
 */
PUBLIC void mpi_group_set_rank(mpi_group_t * group, mpi_process_t * proc)
{
	uassert(group != NULL);

	group->my_rank = MPI_UNDEFINED;

	/* Loop over the group to assert that proc participates on it. */
	if (proc != NULL)
	{
		for (int i = 0; i < group->size; ++i)
		{
			if (proc == group->procs[i])
			{
				group->my_rank = i;
				break;
			}
		}
	}
}

/**
 * @brief Initializes the groups submodule.
 *
 * @returns Upon successful completion, zero is returned. A
 * negative error code is returned instead.
 */
PUBLIC int mpi_group_init(void)
{
	/* Initializes mpi_group_empty. */
	OBJ_CONSTRUCT(&_mpi_group_empty, mpi_group_t);
	_mpi_group_empty.my_rank = MPI_UNDEFINED;
	_mpi_group_empty.size    = 0;

	/* Initializes mpi_group_null. */
	OBJ_CONSTRUCT(&_mpi_group_null, mpi_group_t);
	_mpi_group_null.my_rank = MPI_PROC_NULL;
	_mpi_group_null.size    = 0;

	return (0);
}

/**
 * @brief Finalizes the groups submodule.
 *
 * @returns Upon successful completion, zero is returned. A
 * negative error code is returned instead.
 */
PUBLIC int mpi_group_finalize(void)
{
	/* Destructs the predefined groups. */
	OBJ_DESTRUCT(&_mpi_group_null);

	OBJ_DESTRUCT(&_mpi_group_empty);

	return (0);
}
