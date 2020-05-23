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
#include <mputil/proc.h>
#include <mputil/ptr_array.h>

PRIVATE void process_construct(mpi_process_t *);
PRIVATE void process_destruct(mpi_process_t *);

OBJ_CLASS_INSTANCE(mpi_process_t, &process_construct, &process_destruct, sizeof(mpi_process_t));

/**
 * @brief Global processes variable.
 *
 * @note Instead a #define, this variable should be initialized in execution time.
 */
static int _processes_nr = PROCESSOR_CLUSTERS_NUM;

/**
 * @brief Processes list.
 */
PRIVATE pointer_array_t _processes_list;

/**
 * @brief Local process reference.
 */
PRIVATE mpi_process_t * _local_proc = NULL;

/**
 * @brief Process constructor.
 */
PRIVATE void process_construct(mpi_process_t *proc)
{
	uassert(proc != NULL);

    proc->nodenum = -1;
    proc->pid     = -1;
}

/**
 * @brief Process destructor.
 */
PRIVATE void process_destruct(mpi_process_t *proc)
{
    uassert(proc != NULL);

    /* Removes the process from processes list. */
    if (proc->pid >= 0)
    	uassert(pointer_array_remove(&_processes_list, proc->pid) == 0);
}

/**
 * @brief Allocates a new process for @p nodeid.
 *
 * @param nodeid Process node number.
 *
 * @returns Upon successful completion, the index of the new
 * process in processes_list table is returned. A negative
 * error code is returned instead.
 */
PUBLIC int process_allocate(int nodeid)
{
	int ret;
	mpi_process_t * proc;

	/* Bad node id. */
	if (!WITHIN(nodeid, 0, PROCESSOR_CLUSTERS_NUM))
		return (-EINVAL);

	/* Allocates a new process object. */
	proc = OBJ_NEW(mpi_process_t);
	if (proc == NULL)
		return (-ENOMEM);

	/* Inserts the process in the processes list. */
	if ((ret = pointer_array_insert(&_processes_list, proc)) < 0)
		OBJ_RELEASE(proc);

	/* Initializes the process info. */
	proc->pid     = ret;
	proc->nodenum = nodeid;
	usprintf(proc->name, "nanvix-process-%d", nodeid);

	return (ret);
}

/**
 * @brief Gets reference pointer to the local process.
 *
 * @returns Pointer to the local process descriptor.
 */
PUBLIC mpi_process_t * process_local(void)
{
	return (_local_proc);
}

/**
 * @brief Make a name lookup on the processes list.
 *
 * @param name Name of process to lookup.
 *
 * @returns Upon successful completion. a pointer to the process
 * descriptor is returned. Upon failure, a NULL pointer is returned
 * instead.
 *
 * @note If the name could not be found, a NULL pointer is returned.
 */
PUBLIC mpi_process_t * process_lookup(const char* name)
{
	int limit;
	mpi_process_t * proc;

	limit = pointer_array_get_max_size(&_processes_list);

	for (int i = 0; i < limit; ++i)
	{
		proc = (mpi_process_t *) pointer_array_get_item(&_processes_list, i);
		if (proc == NULL)
			continue;

		if (!ustrcmp(name, proc->name))
			return (proc);
	}

	return (NULL);
}

/**
 * @brief Gets the number of processes active.
 *
 * @returns The number of active processes.
 */
PUBLIC int mpi_proc_count(void)
{
	return (_processes_nr);
}

/**
 * @brief Initializes the processes submodule.
 *
 * @returns Upon successful completion, zero is returned. A
 * negative error code is returned instead.
 */
PUBLIC int mpi_proc_init(void)
{
	int ret; /* Function return. */

	/* Initializes the processes list. */
	OBJ_CONSTRUCT(&_processes_list, pointer_array_t);

	ret = pointer_array_init(&_processes_list, 
		                     TRUNCATE(_processes_nr, 4), 4);
	if (ret != 0)
		goto error;

	/* Allocate and initializes the processes references. */
	for (int i = 0; i < _processes_nr; ++i)
		uassert(process_allocate(i) == i);

	/* Initializes local proc reference. */
	_local_proc = pointer_array_get_item(&_processes_list, cluster_get_num());

	uassert(_local_proc != NULL);

	return (0);

error:
	OBJ_DESTRUCT(&_processes_list);
	return (ret);
}

/**
 * @brief Finalizes the processes submodule.
 *
 * @returns Upon successful completion, zero is returned. A
 * negative error code is returned instead.
 */
PUBLIC int mpi_proc_finalize(void)
{
	int limit;
	mpi_process_t * proc;

	/* Releases local process reference. */
	_local_proc = NULL;

	limit = pointer_array_get_max_size(&_processes_list);

	/* Frees the initialized processes. */
	for (int i = 0; i < limit; ++i)
	{
		proc = (mpi_process_t *) pointer_array_get_item(&_processes_list, i);

		/* Since we don't know the object refcount, we release it the necessary times. */
		while (proc != NULL)
			OBJ_RELEASE(proc);

		/* All processes released. */
		if (pointer_array_get_size(&_processes_list) == 0)
			break;
	}

	/* Releases the processes list. */
	OBJ_DESTRUCT(&_processes_list);

	return (0);
}
