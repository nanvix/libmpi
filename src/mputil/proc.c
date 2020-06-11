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

#include <nanvix/hal.h>
#include <nanvix/ulib.h>
#include <nanvix/runtime/barrier.h>
#include <nanvix/runtime/pm.h>
#include <mputil/proc.h>
#include <mputil/ptr_array.h>
#include <mpi.h>

PRIVATE void process_construct(mpi_process_t *);
PRIVATE void process_destruct(mpi_process_t *);

OBJ_CLASS_INSTANCE(mpi_process_t, &process_construct, &process_destruct, sizeof(mpi_process_t));

/**
 * @brief Global processes variable.
 *
 * @note Instead a #define, this variable should be initialized in execution time.
 */
PRIVATE int _processes_nr = NANVIX_PROC_MAX;

PRIVATE int _active_nodes[NANVIX_PROC_MAX];

/* @note Const barrier parameter workaround. */
PRIVATE const int *_active_nodes_addr = _active_nodes;

PRIVATE barrier_t _std_barrier = BARRIER_NULL;

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

	proc->pid = -1;
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
 * @see process_allocate in proc.h.
 */
PUBLIC int process_allocate(void)
{
	int ret;
	mpi_process_t *proc;

	/* Allocates a new process object. */
	proc = OBJ_NEW(mpi_process_t);
	if (proc == NULL)
		return (-MPI_ERR_NO_MEM);

	/* Inserts the process in the processes list. */
	if ((ret = pointer_array_insert(&_processes_list, (void *) proc)) < 0)
	{
		OBJ_RELEASE(proc);
		return (ret);
	}

	/* Initializes the process info. */
	proc->pid = ret;
	usprintf(proc->name, "mpi-process-%d", ret);

	return (ret);
}

/**
 * @see process_local in proc.h.
 */
PUBLIC mpi_process_t * process_local(void)
{
	return (_local_proc);
}

/**
 * @see mpi_proc_world_list in proc.h.
 */
PUBLIC mpi_process_t ** mpi_proc_world_list(int *size)
{
	int limit;
	mpi_process_t *proc;
	mpi_process_t **procs;

	/* Sanity check. */
	uassert(_processes_nr == pointer_array_get_size(&_processes_list));

	/* Allocates the processes list. */
	procs = (mpi_process_t **) umalloc(_processes_nr * sizeof(mpi_process_t *));
	if (procs == NULL)
		return (NULL);

	limit = pointer_array_get_max_size(&_processes_list);

	for (int i = 0, j = 0; (i < limit) && (j < _processes_nr); ++i)
	{
		proc = (mpi_process_t *) pointer_array_get_item(&_processes_list, i);

		/* Check if there is a valid process with @p i PID. */
		if (proc == NULL)
			continue;

		procs[j++] = proc;
	}

	*size = _processes_nr;

	return (procs);
}

/**
 * @see mpi_proc_self_list in proc.h.
 */
PUBLIC mpi_process_t ** mpi_proc_self_list(int *size)
{
	mpi_process_t **procs;

	procs = (mpi_process_t **) umalloc(sizeof(mpi_process_t *));
	if (procs == NULL)
		return (NULL);

	*procs = process_local();
	*size  = 1;

	return (procs);
}

/**
 * @see mpi_proc_count in proc.h.
 */
PUBLIC int mpi_proc_count(void)
{
	return (_processes_nr);
}

/**
 * @see mpi_std_fence() in proc.h.
 */
PUBLIC int mpi_std_fence(void)
{
	/* Checks if the proc system was already initialized. */
	if (!BARRIER_IS_VALID(_std_barrier))
		return (-EINVAL);

	return (barrier_wait(_std_barrier));
}

/**
 * @see mpi_proc_init in proc.h.
 */
PUBLIC int mpi_proc_init(void)
{
	int ret;       /* Function return.   */
	int local_pid; /* Local process PID. */

	/* Initializes list of nodes for stdbarrier. */
	for (int i = 0; i < _processes_nr; ++i)
		_active_nodes[i] = MPI_PROCESSES_COMPENSATION + i;

	/* Initializes the std_barrier. */
	_std_barrier = barrier_create(_active_nodes_addr, _processes_nr);
	if (!BARRIER_IS_VALID(_std_barrier))
		return (MPI_ERR_NO_MEM);

	/* Initializes the processes list. */
	OBJ_CONSTRUCT(&_processes_list, pointer_array_t);

	ret = pointer_array_init(&_processes_list,
		                     TRUNCATE(_processes_nr, 4), 4);
	if (ret != 0)
		goto error;

	/* Allocate and initializes the processes references. */
	for (int i = 0; i < _processes_nr; ++i)
		uassert(process_allocate() == i);

	/* Calculates local MPI process number. */
	local_pid = cluster_get_num() - MPI_PROCESSES_COMPENSATION;

	/* Initializes local proc reference. */
	_local_proc = (mpi_process_t *) pointer_array_get_item(&_processes_list, local_pid);

	uassert(_local_proc != NULL);

	/* Register the local process in the system distributed lookup table. */
	if ((ret = nanvix_setpname(_local_proc->name)) < 0)
		goto error;

	return (0);

error:
	OBJ_DESTRUCT(&_processes_list);

	barrier_destroy(_std_barrier);

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
	int ret;
	int limit;
	mpi_process_t *proc;

	/**
	 * Unlinks local_proc in system lookup table.
	 *
	 * @note If an error occurs during unlink, proc_finalize will conclude
	 * normally but the error will be reflected on the function return.
	 */
	ret = name_unlink(_local_proc->name);

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

	/* Releases _std_barrier. */
	barrier_destroy(_std_barrier);

	return (ret);
}
