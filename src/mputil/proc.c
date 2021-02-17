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
#include <nanvix/sys/thread.h>
#include <nanvix/runtime/barrier.h>
#include <nanvix/runtime/fence.h>
#include <nanvix/runtime/pm.h>
#include <mputil/proc.h>
#include <mputil/ptr_array.h>
#include <mpi/mpiruntime.h>
#include <nanvix/runtime/stdikc.h>

/**
 * @brief Enable/Disable debug mode.
 */
#define DEBUG 0

/**
 * MPI Class declaration.
 */
PRIVATE void process_construct(mpi_process_t *);
PRIVATE void process_destruct(mpi_process_t *);

OBJ_CLASS_INSTANCE(mpi_process_t, &process_construct, &process_destruct, sizeof(mpi_process_t));

/**
 * @brief Global processes variable.
 *
 * @note Instead a #define, this variable should be initialized in execution time.
 */
PRIVATE int _processes_nr = MPI_PROCESSES_NR;

PRIVATE int _active_nodes[MPI_NODES_NR];

/* @note Const barrier parameter workaround. */
PRIVATE const int *_active_nodes_addr = _active_nodes;

PRIVATE barrier_t _std_barrier;

PRIVATE struct fence_t _std_fence;

/**
 * @brief Processes list.
 */
PRIVATE pointer_array_t _processes_list;

/**
 * @brief Local processes reference.
 */
PRIVATE mpi_process_t *_local_processes[MPI_PROCS_PER_CLUSTER_MAX] = {
	[0 ... MPI_PROCS_PER_CLUSTER_MAX - 1] = NULL
};

/**
 * @brief Number of processes active in the current cluster.
 */
PRIVATE int _local_processes_nr = 0;

/**
 * @brief Master thread ID.
 */
PRIVATE int _master_tid = -1;

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
	proc->tid = -1;

	return (ret);
}

/**
 * @see curr_mpi_proc in proc.h.
 */
PUBLIC mpi_process_t * curr_mpi_proc(void)
{
	int tid; /* Current thread ID. */

	/* Checks if there is only a single process in the current cluster. */
	if (_local_processes_nr == 1)
		return (_local_processes[0]);

	tid = kthread_self();

	/* Gets the process reference associated with the current TID. */
	for (int i = 0; i < _local_processes_nr; ++i)
	{
		if (_local_processes[i]->tid == tid)
			return (_local_processes[i]);
	}

	/* Should never get here. */
	UNREACHABLE();
}

/**
 * @see curr_proc_is_master in proc.h.
 */
PUBLIC int curr_proc_is_master(void)
{
	return ((_local_processes_nr == 1) || (kthread_self() == _master_tid));
}

/**
 * @see curr_mpi_proc_index in proc.h.
 */
PUBLIC int curr_mpi_proc_index(void)
{
	int tid; /* Current thread ID. */

	/* Checks if there is only a single process in the current cluster. */
	if (_local_processes_nr == 1)
		return (0);

	tid = kthread_self();

	/* Gets the process reference associated with the current TID. */
	for (int i = 0; i < _local_processes_nr; ++i)
	{
		if (_local_processes[i]->tid == tid)
			return (i);
	}

	/* Should never get here. */
	UNREACHABLE();
}

/*============================================================================*
 * Special PROC Lists Retrievers                                              *
 *============================================================================*/

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

	*procs = curr_mpi_proc();
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

/*============================================================================*
 * mpi_fence() && mpi_barrier()                                               *
 *============================================================================*/

/**
 * @see mpi_std_fence() in proc.h.
 */
PUBLIC int mpi_std_fence(void)
{
	/* Checks the number of processes locally present. */
	if (_local_processes_nr <= 1)
		goto ret;

	/* Waits in the std_fence. */
	fence(&_std_fence);

ret:
	return (0);
}

/**
 * @see mpi_std_barrier() in proc.h.
 */
PUBLIC int mpi_std_barrier(void)
{
	int ret;

	ret = (-EINVAL);

	/* Checks if the proc system was already initialized. */
	if (_mpi_state < MPI_STATE_INIT_STARTED)
		goto end;

	if ((ret = mpi_std_fence()) != 0)
		goto end;

	/* Master process waits in the dist. barrier to sync with other clusters. */
	if (curr_proc_is_master())
	{
		if ((ret = barrier_wait(_std_barrier)) != 0)
			goto end;
	}

	if ((ret = mpi_std_fence()) != 0)
		goto end;

end:
	return (ret);
}

/*============================================================================*
 * mpi_proc_init()                                                            *
 *============================================================================*/

/**
 * @brief Special struct to carry the traditional main function args for the
 * function wrapper in the pthreads way.
 */
PRIVATE struct
{
	int(*fn)(int, const char *[]);
	int argc;
	const char **argv;
} main_args;

/**
 * @brief Wrapper for the user entry point function that followsthe traditional
 * pthreads functions signature.
 */
PRIVATE void * _main3_wrapper(void *args)
{
	UNUSED(args);

	/* Call main function. */
	main_args.fn(main_args.argc, main_args.argv);

	return (NULL);
}

/**
 * @see mpi_proc_init in proc.h.
 */
PUBLIC int __mpi_processes_init(int(*fn)(int, const char *[]), int argc, const char *argv[])
{
	int ret;             /* Function return.        */
	int tid;             /* Created thread ID.      */
	int first_pid;       /* First Local process ID. */
	int local_processes; /* Test verification.      */

	/* Initializes list of nodes for stdbarrier. */
	for (int i = 0; i < MPI_NODES_NR; ++i)
		_active_nodes[i] = MPI_PROCESSES_COMPENSATION * MPI_NODES_COMPENSATION + i;

	/* Initializes the std_barrier. */
	_std_barrier = barrier_create(_active_nodes_addr, MPI_NODES_NR);
	if (!BARRIER_IS_VALID(_std_barrier))
		return (MPI_ERR_NO_MEM);

	/* Initializes the processes list. */
	OBJ_CONSTRUCT(&_processes_list, pointer_array_t);

	ret = pointer_array_init(&_processes_list,
		                 TRUNCATE(_processes_nr, 4), 4
		                );

	/* Checks if processes list was correctly initialized. */
	if (ret != 0)
		goto error;

	/* Allocate and initializes the processes references. */
	for (int i = 0; i < _processes_nr; ++i)
		uassert(process_allocate() == i);

	/* Calculates the lowest local MPI process number. */
	first_pid = kcluster_get_num() - MPI_PROCESSES_COMPENSATION;

	/* Specifies the current thread as the master of the current cluster. */
	_master_tid = kthread_self();

	/* Calculates how many processes will be locally initialized. */
	_local_processes_nr = ((int) (MPI_PROCESSES_NR / MPI_NODES_NR)) +
				 ((first_pid < (MPI_PROCESSES_NR % MPI_NODES_NR)) ? 1 : 0
			      );

	/* Initializes the local_process[0]. */
	_local_processes[0] = (mpi_process_t *) pointer_array_get_item(&_processes_list, first_pid);
	uassert(_local_processes[0] != NULL);

	_local_processes[0]->tid = _master_tid;

	/* Checks if more than a single process will be spawned in the current cluster. */
	if (_local_processes_nr > 1)
	{
		/* THREADS ARE NECESSARY TO EMULATE PROCESSES. */

		local_processes = 1;

		/* Initializes the std_fence. */
		fence_init(&_std_fence, _local_processes_nr);

		/* Initializes the parameters passing structure. */
		main_args.fn = fn;
		main_args.argc = argc;
		main_args.argv = argv;

		/* Initializes the local processes list and create the threads that will run them. */
		for (int i = 1, id = (first_pid + MPI_NODES_NR); id < _processes_nr; id += MPI_NODES_NR, ++i)
		{
			_local_processes[i] = (mpi_process_t *) pointer_array_get_item(&_processes_list, id);
			uassert(_local_processes[i] != NULL);

			/* Creates a thread to emulate an MPI process. */
			uassert(kthread_create(&tid, &_main3_wrapper, NULL) == 0);

			_local_processes[i]->tid = tid;

			local_processes++;
		}

		uassert(local_processes == _local_processes_nr);
	}

	return (0);

error:
	OBJ_DESTRUCT(&_processes_list);

	barrier_destroy(_std_barrier);

	return (ret);
}

/**
 * @see mpi_local_proc_init in proc.h.
 */
PUBLIC int mpi_local_proc_init(void)
{
	int ret;                  /* Function return.           */
	int mbxid;
	int portalid;
	mpi_process_t *curr_proc; /* Current process reference. */
	const char *curr_proc_name;

	curr_proc = curr_mpi_proc();
	curr_proc_name = process_name(curr_proc);

#if DEBUG
	uprintf("%s creating inbox", curr_proc_name);
#endif /* DEBUG */

	/* Initializes input mailbox. */
	if ((mbxid = nanvix_mailbox_create(curr_proc_name)) < 0)
	{
		ret = mbxid;
		goto err0;
	}

#if DEBUG
	uprintf("%s creating inportal", curr_proc_name);
#endif /* DEBUG */

	/* Initializes input portal. */
	if ((portalid = nanvix_portal_create(curr_proc_name)) < 0)
	{
		ret = portalid;
		goto err1;
	}

#if DEBUG
	uprintf("%s registering local port %d", curr_proc_name, nanvix_mailbox_get_port(mbxid));
#endif /* DEBUG */

	/* Registers the local process in the system name service. */
	if ((ret = nanvix_name_register(curr_proc_name, nanvix_mailbox_get_port(mbxid))) < 0)
		goto err2;

	/**
	 * @brief Security check.
	 */
	uassert(nanvix_mailbox_get_port(mbxid) == nanvix_portal_get_port(portalid));

	/* Updates curr_proc info. */
	curr_proc->inbox = mbxid;
	curr_proc->inportal = portalid;

#if DEBUG
	uprintf("%s inbox: %d", curr_proc_name, mbxid);
	uprintf("%s inportal: %d", curr_proc_name, portalid);
#endif /* DEBUG */

	return (0);

err2:
	uassert(nanvix_portal_unlink(portalid) == 0);

err1:
	uassert(nanvix_mailbox_unlink(mbxid) == 0);

err0:
	return (ret);
}

/*============================================================================*
 * mpi_proc_finalize()                                                        *
 *============================================================================*/

/**
 * @brief Finalizes the processes submodule.
 *
 * @returns Upon successful completion, zero is returned. A
 * negative error code is returned instead.
 */
PUBLIC int __mpi_processes_finalize(void)
{
	int limit;
	mpi_process_t *proc;

	/* Joins the user threads previously spawned in processes_init. */
	for (int i = (_local_processes_nr - 1); i > 0; --i)
	{
		uprintf("FINALIZING THREAD %d", i);

		uassert(kthread_join(_local_processes[i]->tid, NULL) == 0);

		uprintf("THREAD FINALIZED");

		_local_processes[i] = NULL;

		_local_processes_nr--;
	}

	/* Security check. */
	uassert(_local_processes_nr == 1);

	/* Releases local process[0] reference. */
	_local_processes[0] = NULL;

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

	return (0);
}

/**
 * @see mpi_local_proc_finalize in proc.h.
 */
PUBLIC int mpi_local_proc_finalize(void)
{
#if DEBUG
	uprintf("Thread %d Unregistering", kthread_self());
#endif /* DEBUG */

	/* Unregisters the local process from the system name service. */
	uassert(nanvix_name_unregister(process_name(curr_mpi_proc())) == 0);

#if DEBUG
	uprintf("Thread %d unlinking portal", kthread_self());
#endif /* DEBUG */

	/* Unlinks the inportal of the local process. */
	uassert(nanvix_portal_unlink(curr_mpi_proc_inportal()) == 0);

#if DEBUG
	uprintf("Thread %d unlinking mailbox", kthread_self());
#endif /* DEBUG */

	/* Unlinks the inbox of the local process. */
	uassert(nanvix_mailbox_unlink(curr_mpi_proc_inbox()) == 0);

#if DEBUG
	uprintf("Thread %d finalized", kthread_self());
#endif /* DEBUG */

	return (0);
}

/*============================================================================*
 * mpi_local_procs_nr()                                                       *
 *============================================================================*/

 /**
 * @brief Gets the number of locally present MPI processes.
 *
 * @returns The number of MPI processes that are locally present.
 */
PUBLIC int mpi_local_procs_nr(void)
{
	return (_local_processes_nr);
}
